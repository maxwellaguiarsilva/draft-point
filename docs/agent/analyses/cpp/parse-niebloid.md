# The `parse` Niebloid

The second design step of the original `value_or` + `parse` proposal, split into this document and [value-or-indexed-overload.md](value-or-indexed-overload.md). This part records the **reflection**: evaluating a `parse` niebloid that encapsulates `from_chars`, generalizing the earlier scalar-only design into a scalar-and-range parser with an optional fallback, which composes with `value_or` to fully remove the parsing machinery from the test.

## 5. Reflection: `parse` Niebloid

### 5.1 Motivation

The `from_chars` + full-consumption + `errc` check is generic text-conversion logic that recurs in every adhoc test that reads numeric arguments, whether it consumes a single token or a whole container of tokens. Per the project priorities (DRY first), it should be a named `sak` utility rather than inline machinery (DDD: "parse argument 1 as byte, default 8" instead of `from_chars` plumbing). The same primitive therefore covers the scalar case directly and the range case by element-wise application.

### 5.2 Core primitive and helpers

Every entry point funnels into a single shared helper, `__parse_scalar`, which parses one `string_view` with `std::from_chars` and requires whole consumption:

```cpp
template< is_number t_number >
constexpr auto __parse_scalar( const string_view text, const t_number default_value ) noexcept -> t_number
{
	t_number result{ default_value };
	const auto [ pointer, error ] = from_chars( text.data( ), text.data( ) + text.size( ), result );
	return	pointer == text.data( ) + text.size( ) and error == errc{ } ? result : default_value;
}
```

Notes:

- **Whole consumption:** trailing garbage (`"8abc"`) fails, matching the previous behavior.
- **`errc{ }` check:** a range failure on the target type yields the fallback.
- **`constexpr`:** `from_chars` on integer types is `constexpr` in C++23, so `parse` can be `constexpr` (libstdc++ supports it), enabling compile-time default parsing.
- **Fallback handling:** a bound fallback is carried by `__parse_default_present`, while an absent fallback is represented by `__parse_default_absent`, whose deferred conversion yields `t_number{ }` (the zero value). An element parser, `__parse_element`, binds one fallback value so it can be reused across a range, and a deferred proxy, `__parse_proxy`, resolves the conversion to either a scalar or a container target.

### 5.3 Constraint: `is_number`

`from_chars` does not support `bool`, so a dedicated `is_number` concept excludes it; the requirement is reused from `sak::math::is_number`:

```cpp
template< typename t_number >
concept is_number	=	is_arithmetic< t_number > and not same_as< t_number, bool >;
```

### 5.4 Scalar interface

There are three scalar forms, all backed by `__parse_scalar`:

- **Direct with fallback:** the target type is deduced from the fallback argument.

```cpp
int value = parse( string{ "42" }, 0 );
```

- **Piped, target from context:** the target type is deduced from the assignment site through a deferred conversion; an invalid input yields the zero value.

```cpp
int value = text | parse;
```

- **Piped with fallback:** the fallback is bound first and carried by the closure.

```cpp
int value = text | parse( 0 );
```

### 5.5 Range interface

When the source is a list of text elements rather than one contiguous buffer, `parse` applies the same primitive element-wise and materializes the result. Range materialization reuses `sak::ranges::to`, and the element-wise application reuses `sak::ranges::lazy_transform`:

```cpp
vector< int > list = text_list | parse;
array< byte, 3 > list = text_raw | parse( byte{ 9 } );
```

The distinction is constrained by the source shape: a `__text_range` is a contiguous, sized range of `char` and resolves to a scalar target, whereas a `__text_list` is any input range whose elements are convertible to `string_view` and resolves to a container target. The fallback remains optional in the range form too.

### 5.6 Range validation stays in the caller

`between( parsed, 1, 255 )` rejects `0` (a degenerate sphere count), which is domain logic. `parse` must not encode it. However, note that parsing directly into `byte` (`uint8_t`) changes semantics: `from_chars` would accept `0`, which the original code rejects. To preserve behavior, keep the caller's range check.

### 5.7 Composition with `value_or`

`parse` subsumes the conversion half of the block; `value_or` subsumes the lookup half. Composed with a sentinel default of `0` (which the range check rejects), the whole block collapses to:

```cpp
const string total_default{ "8" };
const int parsed = parse( value_or( arguments, 1uz, total_default ), 0 );
const byte total = between( parsed, 1, 255 ) ? parsed : 8;
```

Note the explicit template argument is omitted: `parse` deduces `t_number` from the default argument (`0`, an `int`). Likewise, no `static_cast< byte >` is needed — the declared type `byte` drives the implicit narrowing, which compiles clean under `-Wall -Wextra -Werror` (verified). The `static_cast` was strictly unnecessary per the [C++ Style Guide](../../../../../project-mcp-tools/docs/agent/style-guide/cpp.md) "Casts" rule.

Behavior matches the original exactly:

| Input | `value_or` | `parsed` | `between( 1, 255 )` | `total` |
|---|---|---|---|---|
| no argument | `"8"` | `8` | yes | `8` |
| `"5"` | `"5"` | `5` | yes | `5` |
| `"0"` | `"0"` | `0` | no | `8` |
| `"300"` | `"300"` | `300` | no | `8` |
| `"8abc"` | `"8abc"` | `0` | no | `8` |

### 5.8 Placement

`parse` is generic and domain-independent; it belongs in `sak`. It is placed at `sak/pattern/parse.hpp` (alongside `value_or`), treating conversion as a pattern.

### 5.9 Rationale for the name and for not merging into `sak::ranges::to`

`sak::ranges::to` is a range materializer: its role is to turn a pipeline into a container, and its name is already bound to that deferred-conversion protocol. `parse` also performs a deferred conversion, but its semantics are text interpretation (with an optional fallback), not range materialization. Folding it into `to` would collide with, and overload against, the materializer, producing exactly the overload ambiguity the previous scalar name risked; naming the primitive `parse` keeps the two concerns and their overload sets distinct. The full interface lives in one header, `include/sak/pattern/parse.hpp`.

---

## 6. Properties

| Property | Description |
|---|---|
| **DRY** | The `from_chars` machinery lives once in `__parse_scalar` instead of in every test. |
| **DDD** | `parse( value, default_value )` states intent, not the parse algorithm. |
| **Composable** | `value_or` (lookup) and `parse` (conversion) address orthogonal concerns and compose in a single expression. |
| **Uniform** | One primitive serves both a single text token and a list of text elements, with or without a fallback. |
| **Semantics-preserving** | The composed form reproduces the original table exactly, including the `0` rejection and trailing-garbage failure. |
| **No temporaries** | Both utilities accept only lvalue defaults (`const&` returns), forcing the caller to declare defaults on the stack. |
