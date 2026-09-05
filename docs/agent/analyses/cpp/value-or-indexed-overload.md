# `value_or` Indexed Overload

The first design step of the original
[`value_or` + `to_number` proposal](../cpp/value-or-indexed-and-to-number.md),
split into this document and [to-number-niebloid.md](to-number-niebloid.md).
This part records the **plan**: extending `sak::pattern::value_or` with an
overload for indexed containers.

## 1. Objective

Refactor the `$1` argument parsing in `tests/adhoc/rendering/0010_sphere.cpp` so the
`if ( arguments.size( ) > 1 )` guard disappears, the default value is declared on the
stack, and the access is expressed as a lookup. The conversion and range validation
halves are handled by `to_number` ([to-number-niebloid.md](to-number-niebloid.md)).

---

## 2. Current Code

```cpp
const vector< string > arguments( argument_values, argument_values + argument_count );
byte total = 8;
if( arguments.size( ) > 1 )
{
	int parsed = 0;
	const auto& value = arguments[ 1 ];
	const auto [ pointer, error ] = from_chars( value.data( ), value.data( ) + value.size( ), parsed );
	if( pointer == value.data( ) + value.size( ) and error == errc{ } and between( parsed, 1, 255 ) )
		total = static_cast< byte >( parsed );
}
```

The block mixes three concerns: bounded index access, string-to-number conversion, and
domain range validation. The objective is to replace the size guard with a lookup.

---

## 3. Plan: Indexed Overload of `value_or`

### 3.1 Why the existing `value_or` does not fit

`include/sak/pattern/value_or.hpp` models only associative containers:

```cpp
const auto iterator = container.find( key );
return	iterator not_eq container.end( ) ? iterator->second : default_value;
```

It requires `key_type` and `mapped_type`. A `vector< string >` has neither, so the lookup
`arguments[ 1 ]` cannot be expressed through it.

### 3.2 Overloaded functor

The plan adds a second `operator()` to `__value_or`, constrained with `requires` clauses
so the associative and indexed paths are disambiguated purely by the container signature:

```cpp
struct __value_or
{
	template< typename t_container >
	requires( requires( const t_container& container, const typename t_container::key_type& key ) { container.find( key ); } )
	constexpr auto operator ( ) ( const t_container& container, const typename t_container::key_type& key, const typename t_container::mapped_type& default_value ) const noexcept -> const typename t_container::mapped_type&
	{
		const auto iterator = container.find( key );
		return	iterator not_eq container.end( ) ? iterator->second : default_value;
	}

	template< typename t_container >
	requires( not requires( const t_container& container, const typename t_container::key_type& key ) { container.find( key ); }
		and requires( const t_container& container, const typename t_container::size_type& index ) { container[ index ]; } )
	constexpr auto operator ( ) ( const t_container& container, const typename t_container::size_type& index, const typename t_container::value_type& default_value ) const noexcept -> const typename t_container::value_type&
	{
		return	index < container.size( ) ? container[ index ] : default_value;
	}
};
inline constexpr auto value_or = __value_or{ };
```

### 3.3 Disambiguation

- The associative overload is enabled only when `container.find( key )` is valid.
- The indexed overload requires `not find` **and** a valid `operator[ ]` with `size_type`.
  For `vector`, the `key_type` reference in the first overload is ill-formed inside the
  `requires`-expression, which evaluates to `false` (SFINAE-friendly), so only the
  indexed overload remains viable.
- The `not find` clause prevents `std::map` from accidentally binding to the indexed
  path via its own `operator[ ]`.

---

## 4. Usage in `0010_sphere.cpp`

`sak/sak.hpp` does not aggregate `pattern/value_or.hpp`, so the test must include it
explicitly and bring the symbol in:

```cpp
#include <sak/pattern/value_or.hpp>
// ...
__using( ::sak::pattern::, value_or )
```

```cpp
const string total_default{ "8" };
const string total_value = value_or( arguments, 1uz, total_default );
byte total = 8;
int parsed = 0;
const auto [ pointer, error ] = from_chars( total_value.data( ), total_value.data( ) + total_value.size( ), parsed );
if( pointer == total_value.data( ) + total_value.size( ) and error == errc{ } and between( parsed, 1, 255 ) )
	total = static_cast< byte >( parsed );
```

The size guard is gone; the default is a stack lvalue (no temporaries are accepted, since
the functor returns `const&`). The conversion and validation machinery remain — those
move to `to_number` in the [companion document](to-number-niebloid.md).
