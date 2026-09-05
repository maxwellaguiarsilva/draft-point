# The `to_number` Niebloid

The second design step of the original
[`value_or` + `to_number` proposal](../cpp/value-or-indexed-and-to-number.md),
split into this document and [value-or-indexed-overload.md](value-or-indexed-overload.md).
This part records the **reflection**: evaluating a `to_number` niebloid that encapsulates
`from_chars`, which composes with `value_or` to fully remove the parsing machinery from
the test.

## 5. Reflection: `to_number` Niebloid

### 5.1 Motivation

The `from_chars` + full-consumption + `errc` check is generic string-to-number logic that
will recur in every adhoc test that reads numeric arguments. Per the project priorities
(DRY first), it should be a named `sak` utility rather than inline machinery (DDD:
"parse argument 1 as byte, default 8" instead of `from_chars` plumbing).

### 5.2 Proposed signature and semantics

```cpp
to_number< is_number t_number >( const string& value, const t_number default_value )
```

Notes:

- `default` is a C++ keyword, so the parameter is `default_value`.
- Following the existing `sak` niebloid pattern (`inline constexpr auto name = __name{ };`,
  no variable templates anywhere in `sak`), `to_number` is a plain functor whose
  `operator()` is constrained by `is_number`. The target type is deduced from the default
  argument, so the call is `to_number( value, 0 )`:

```cpp
struct __to_number
{
	template< is_number t_number >
	constexpr auto operator ( ) ( const string& value, const t_number default_value ) const noexcept -> t_number
	{
		t_number result{ default_value };
		const auto [ pointer, error ] = from_chars( value.data( ), value.data( ) + value.size( ), result );
		return	pointer == value.data( ) + value.size( ) and error == errc{ } ? result : default_value;
	}
};
inline constexpr auto to_number = __to_number{ };
```

- **Full consumption:** trailing garbage (`"8abc"`) fails, matching the current behavior.
- **`errc{ }` check:** a range failure on the target type yields `default_value`.
- **`constexpr`:** `from_chars` on integer types is `constexpr` in C++23, so `to_number`
  can be `constexpr` (libstdc++ supports it), enabling compile-time default parsing.

### 5.3 Constraint: `is_number`

The existing `sak::math::is_arithmetic` is too broad: it admits `bool`, which
`from_chars` does not support. A dedicated `is_number` should exclude `bool`:

```cpp
template< typename t_number >
concept is_number	=	is_arithmetic< t_number > and not same_as< t_number, bool >;
```

### 5.4 Range validation stays in the caller

`between( parsed, 1, 255 )` rejects `0` (a degenerate sphere count), which is domain
logic. `to_number` must not encode it. However, note that parsing directly into `byte`
(`uint8_t`) changes semantics: `from_chars` would accept `0`, which the original code
rejects. To preserve behavior, keep the caller's range check.

### 5.5 Composition with `value_or`

`to_number` subsumes the conversion half of the block; `value_or` subsumes the lookup
half. Composed with a sentinel default of `0` (which the range check rejects), the whole
block collapses to:

```cpp
const string total_default{ "8" };
const int parsed = to_number( value_or( arguments, 1uz, total_default ), 0 );
const byte total = between( parsed, 1, 255 ) ? parsed : 8;
```

Note the explicit template argument is omitted: `to_number` deduces `t_number` from the
default argument (`0`, an `int`). Likewise, no `static_cast< byte >` is needed — the
declared type `byte` drives the implicit narrowing, which compiles clean under
`-Wall -Wextra -Werror` (verified). The `static_cast` was strictly unnecessary per the
[C++ Style Guide](../../../../../project-mcp-tools/docs/agent/style-guide/cpp.md) "Casts" rule.

Behavior matches the original exactly:

| Input | `value_or` | `parsed` | `between( 1, 255 )` | `total` |
|---|---|---|---|---|
| no argument | `"8"` | `8` | yes | `8` |
| `"5"` | `"5"` | `5` | yes | `5` |
| `"0"` | `"0"` | `0` | no | `8` |
| `"300"` | `"300"` | `300` | no | `8` |
| `"8abc"` | `"8abc"` | `0` | no | `8` |

### 5.6 Placement

`to_number` is generic and domain-independent; it belongs in `sak`. Two options:

- `sak/pattern/to_number.hpp` (alongside `value_or`) — treats conversion as a pattern.
- A new `sak/convert/` directory — groups conversion utilities.

---

## 6. Properties

| Property | Description |
|---|---|
| **DRY** | The `from_chars` machinery lives once in `to_number` instead of in every test. |
| **DDD** | `to_number< byte >( value, default_value )` states intent, not the parse algorithm. |
| **Composable** | `value_or` (lookup) and `to_number` (conversion) address orthogonal concerns and compose in a single expression. |
| **Semantics-preserving** | The composed form reproduces the original table exactly, including the `0` rejection and trailing-garbage failure. |
| **No temporaries** | Both utilities accept only lvalue defaults (`const&` returns), forcing the caller to declare defaults on the stack. |
