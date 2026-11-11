# Point Niebloid Purification

## Scope

- Canonical architecture of `sak::point< t_scalar, num_dimensions >` (`include/sak/geometry/point.hpp`) and its external `sak` operation surface:
  - `sak::point` owns fixed-dimension contiguous coordinate storage (`num_dimensions > 1`) and standard range-container compatibility.
  - Value equality (`operator ==`) is the sole operation declared on `point`, delegating directly to `sak::is_equal`.
  - Dimension-symmetric operations live outside `point` as free `sak` niebloids and range operators:
    - Coordinate predicates:
      - `is_all`
      - `is_inside_of`
      - `is_equal`
    - Euclidean vector norms (`length`).
    - Scalar coordinate reductions (`product`).
    - Element-wise arithmetic range operators.

## Class Surface (`include/sak/geometry/point.hpp`)

### Storage and Range Interface

- Storage base: `private std::array< t_scalar, num_dimensions >` (`super_type`), providing contiguous unpadded storage satisfying `is_flat` while encapsulating `std::array`.
- Exported `super_type` container type aliases (`__using( typename super_type::, ... )`):
  - `value_type`
  - `size_type`
  - `difference_type`
  - `pointer`
  - `const_pointer`
  - `reference`
  - `const_reference`
  - `iterator`
  - `const_iterator`
- Exported `super_type` container accessors (`__using( super_type::, ... )`):
  - `begin`
  - `end`
  - `size`
  - `operator [ ]`
  - `data`
  - `fill`
- Equality operator: `constexpr auto operator == ( const point& other ) const noexcept -> bool`, delegating value-semantic equality to `is_equal( *this, other )`.
- Range conversion target: `sak::ranges::__to_impl< point< t_scalar, num_dimensions > >`, materializing lazy range pipelines via `| to` through `point( from_range, range )`.

### Constructors

| Signature | Constraints | Behavior and Downstream Role |
|---|---|---|
| `constexpr point( )` | `num_dimensions > 1` | Zero-initializes all `num_dimensions` coordinates |
| `constexpr point( t_args... args )` | `sizeof...( t_args ) <= num_dimensions` and each `convertible_to< t_args, t_scalar >` | Stores leading scalar arguments in pack order and zero-initializes remaining coordinates |
| `constexpr point( t_range&& range, t_first first, t_remaining... remaining )` | `sizeof...( t_remaining ) + 1 < num_dimensions`, range + trailing scalars `convertible_to< t_scalar >` | Copies `num_dimensions - trailing_count` prefix coordinates from `range \| take( prefix_size )` followed by `trailing_count` trailing scalars; enables dimension-symmetric assembly of 4D RGBA colors and 4D quaternions `point< float, 4 >( imaginary_part, scalar_part )` in [Agnostic Pipeline](agnostic-pipeline.md) and [Orientation](orientation.md) |
| `constexpr explicit point( const point< t_other_scalar, num_dimensions >& other )` | `convertible_to< t_other_scalar, t_scalar >` and `not same_as< t_other_scalar, t_scalar >` | Explicit element-wise scalar type conversion across equal dimensionality |
| `constexpr explicit point( const point< t_other_scalar, other_dimensions >& other )` | `other_dimensions not_eq num_dimensions` | Explicit cross-dimension projection copying `other \| take( num_dimensions )` |
| `constexpr point( from_range_t, t_range&& range )` | `input_range< t_range >` | Copies `range` into storage as the backing constructor for `sak::ranges::to` |

## External Niebloid and Operator Surface

| Symbol | Namespace and Header | Category | Dimension-Symmetric Definition | Return Semantics |
|---|---|---|---|---|
| `sak::is_all` | `sak` (`include/sak/geometry/point.hpp`) | Binary range predicate niebloid | `all_of( zip( left, right ), tupled( operation ) )` | `bool`; true when every paired coordinate satisfies `operation` |
| `sak::is_inside_of` | `sak` (`include/sak/geometry/point.hpp`) | Binary range predicate niebloid | `is_all( inner, outer, less_equal )` | `bool`; coordinate-wise upper-bound containment |
| `sak::is_equal` | `sak` (`include/sak/geometry/point.hpp`) | Binary range predicate niebloid | `is_all( left, right, equal_to )` | `bool`; coordinate-wise equality across two input ranges |
| `sak::math::length` | `sak::math` (`include/sak/math/vector.hpp`) | Unary range reduction niebloid | `square_root( dot( vector ) )` | Natural floating-point Euclidean norm returned by `square_root` (`sqrt( 14.0 )` for `point< int, 3 >{ 1, 2, 3 }`) |
| `sak::math::product` | `sak::math` (`include/sak/math/math.hpp`) | Unary range reduction niebloid | `fold_left( list, 1, multiplies )` | Scalar product across all elements of a coordinate range |
| Element-wise binary and compound arithmetic operators | `sak::ranges` (`include/sak/ranges/operators.hpp`), re-exported in `sak::math` and `sak` | Macro-generated range operators (`__781083963_binary`) | `zip_transform( operation, all( left ), all( right ) ) \| to` (with scalar right operands broadcast via `repeat`) | Eager `point` when the left operand is a `point` container; lazy view when the left operand is a view |

```cpp
struct __is_all
{
	template< input_range t_left, input_range t_right, typename t_operation >
	constexpr auto operator ( ) ( t_left&& left, t_right&& right, const t_operation& operation ) const noexcept -> bool
	{
		__using( ::std::views::, zip )
		__using( ::std::ranges::, all_of )
		return	all_of( zip( left, right ), tupled( operation ) );
	}
};
inline constexpr auto is_all = __is_all{ };

struct __is_inside_of
{
	constexpr auto operator ( ) ( const auto& inner, const auto& outer ) const noexcept -> bool
	{ return is_all( inner, outer, less_equal ); }
};
inline constexpr auto is_inside_of = __is_inside_of{ };

struct __is_equal
{
	constexpr auto operator ( ) ( const auto& left, const auto& right ) const noexcept -> bool
	{ return is_all( left, right, equal_to ); }
};
inline constexpr auto is_equal = __is_equal{ };

struct __product
{
	constexpr auto operator ( ) ( const auto& list ) const noexcept
	{ return fold_left( list, 1, multiplies ); }
};
inline constexpr auto product = __product{ };

struct __length
{
	constexpr auto operator ( ) ( const auto& vector ) const noexcept
	{ return square_root( dot( vector ) ); }
};
inline constexpr auto length = __length{ };
```

## Placement and Naming Rationale

- Namespace placement:
  - `sak::geometry` (`include/sak/geometry/geometry.hpp`) is a class template parameterized by scalar type and dimension count, so coordinate-range predicates reside in flat `namespace sak` inside `include/sak/geometry/point.hpp`:
    - `is_all`
    - `is_inside_of`
    - `is_equal`
  - Numeric range reductions (`length` and `product`) reside in `namespace sak::math` alongside existing vector and scalar reductions in `include/sak/math/vector.hpp` and `include/sak/math/math.hpp`.
- Collision-free naming:
  - `is_inside_of` distinguishes the free binary coordinate predicate from the receiver-bound member `sak::geometry::rectangle::is_inside` (`include/sak/geometry/geometry.hpp`).
  - `is_equal` distinguishes the whole-range equality fold from the scalar comparator `std::equal_to` (`sak::math::equal_to`).
  - Bare nouns (`length` and `product`) follow the `docs/agent/style-guide/cpp.md` bare-noun naming convention.
- Reduction call semantics and view-chaining rule:
  - Scalar reductions collapse an input range into a single scalar value and are defined as plain function objects:
    - `length`
    - `product`
    - `sum`
    - `dot`
  - `sak::point` is an eager owning container, while whole-vector closures in `include/sak/math/vector.hpp` constrain `operator ( )` on `viewable_range`:
    - `normalize`
    - `cross`
    - `rotate`
  - Chaining multiple whole-vector closures on a `point` value binds an intermediate `const auto` lvalue or starts from a view adaptor (`views::all` or `rotated`).

## Boundary

- `sak::geometry::rectangle` (`include/sak/geometry/geometry.hpp`) retains its domain-specific `contains` and `is_inside` member methods, implemented over `is_all` and `is_inside_of`.
- Geometric primitive evaluators and GPU payload layouts belong to [Agnostic Pipeline](agnostic-pipeline.md).
- 4D quaternion multiplication and 3D basis conversion in [Orientation](orientation.md) compose over:
  - `point< float, 4 >( imaginary_part, scalar_part )`
  - `dot`
  - `cross`
  - `rotated`

## Relations

- Parent domain: [3D Renderer](index.md)
- Direct consumers:
  - [Agnostic Pipeline](agnostic-pipeline.md)
  - [Orientation](orientation.md)
  - [Spatial Graph](spatial-graph.md)
