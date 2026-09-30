# `sak::ranges::to` proxy conversion ambiguity

## Summary

The `to` materializer in `include/sak/ranges/to.hpp` wraps a range in `__to_proxy`, whose conversion operator is an unconstrained template:

```cpp
template< typename t_target >
constexpr operator t_target( ) &&
{
	return	__to_impl< t_target >::apply( ::std::forward< t_range >( m_range ) );
}
```

Because the operator has no `requires` clause, the proxy is considered convertible to *any* type, including arithmetic scalars. While `sak::point` required exactly `num_dimensions` arguments this was harmless, but once `point` accepts fewer than `num_dimensions` scalars, an expression of the form `range | to` assigned to a `point` becomes an ambiguous conversion.

## Symptom

```
error: conversion from 'sak::ranges::__to_proxy<...>' to 'sak::point<float, 3>' is ambiguous
```

## Root cause

For `const point< float, 3 > p = expr | to;` the compiler finds two user-defined conversion paths:

1. `__to_proxy::operator point< float, 3 >( ) &&` — the intended direct materialization through `__to_impl< point< float, 3 > >`.
2. `point< float, 3 >( t_args... )` with `t_args = { proxy }` — newly viable because the constructor now accepts `sizeof...( t_args ) <= num_dimensions` and its constraint is `convertible_to< t_args, t_scalar >`. The unconstrained `operator t_target()` makes `convertible_to< __to_proxy<...>, float >` evaluate to `true` (the operator is a viable candidate at overload-resolution time; its body is not instantiated then, so the failure never surfaces).

Both are user-defined conversion sequences, so the initialization is ambiguous.

## Minimal reproduction

```cpp
using	point3	=	::sak::point< float, 3 >;

const point3 normal	=	cross( edge_first, edge_second ) | to;
```

## Affected call sites

Observed in `pickup-express` after enabling the `<= num_dimensions` constructor:

- `source/pickup/scene/car.cpp:64`
- `source/pickup/mesh/ccf.cpp:1217`
- `source/sdl3_demo.cpp:172`

## Related notes

The scalar-fill constructor body must keep the increment inside a parenthesized comma operand:

```cpp
( ( ( *this )[ index ] = static_cast< t_scalar >( args ), ++index ), ... );
```

Writing the assignment directly as the fold operand (`( *this )[ index++ ] = ...`) triggers `-Werror`:

```
error: binary expression in operand of fold-expression [-Wtemplate-body]
```

## Candidate directions

Not a decision, only options to evaluate in a clean session:

- Constrain `operator t_target()` with a concept, e.g. require `__to_impl< t_target >` to be applicable, or exclude arithmetic targets.
- Make the conversion operator `explicit` (changes every call site).
- Constrain the scalar-fill constructor so its arguments cannot be a range/proxy, e.g. require `is_arithmetic< t_args >` instead of `convertible_to< t_args, t_scalar >`.
- Keep the constructor at `== num_dimensions` and add a distinct, clearly-constrained constructor for the partial-fill case.
