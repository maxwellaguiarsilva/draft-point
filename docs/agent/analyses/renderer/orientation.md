# Orientation

## Scope

- Equips scene objects with 3D orientation stored on the CPU as a 4D unit quaternion (`point< float, 4 >`), composed via `hamilton_product` and converted on the CPU into a 9-float orthonormal basis via `orientation_basis`.
- Keeps quaternion algebra on the CPU while the GPU consumes the orthonormal `mat3` basis columns to transform rays into each figure's local canonical frame.

## Verified `sak` Baseline

- Existing vector rotation surface:
  - `include/sak/math/vector.hpp` provides `sak::math::rotate`, a Rodrigues rotation of a 3D vector around an axis by an angle returning a lazy vector view.
- `sak::math` quaternion and basis surface owned by this specification:
  - `imaginary_part` and `scalar_part` component projections over `quaternion` (`point< float, 4 >`).
  - `hamilton_product` for unit quaternion composition.
  - `quaternion_rotate` for 3D vector rotation by a unit quaternion.
  - `orientation_basis` for orthonormal 3D basis generation (`array< point< float, 3 >, 3 >`).
- Storage and flat-buffer compatibility:
  - `sak::point< t_scalar, num_dimensions >` (`include/sak/geometry/point.hpp`) constrains `t_scalar` with `sak::math::is_arithmetic`, providing explicit cross-dimension truncation `point< float, 3 >( rotation )` and range-plus-trailing-scalar assembly `point< float, 4 >( imaginary_part, scalar_part )`.
  - `std::array< point< float, 3 >, 3 >` satisfies `sak::opengl::is_flat` (`include/sak/opengl/shader.hpp`):
    - Trivially copyable layout with zero padding.
    - Nine contiguous `float` scalars (`36` bytes).

## Premeditated Dimension-Symmetric Formulation in `sak`

### Quaternion Storage Convention

- Type alias and multiplicative identity:
  - `using quaternion = point< float, 4 >;` with multiplicative identity `point< float, 4 >{ 0.0f, 0.0f, 0.0f, 1.0f }`.
- Named component projections isolating the `( 3D imaginary, 1D real )` split in one definition:
  - `imaginary_part( rotation )` extracts the 3D imaginary vector `point< float, 3 >( rotation )` via `point`'s explicit cross-dimension truncation constructor.
  - `scalar_part( rotation )` extracts the real component at index `3` via a single named `__scalar_part` closure mirroring `__axis` in `include/sak/geometry/geometry.hpp`.
- Canonical reassembly constructs every 4D quaternion via `point< float, 4 >( imaginary_part, scalar_part )`.

### Symmetric `hamilton_product`

- Given `left = point< float, 4 >( left_imaginary, left_scalar )` and `right = point< float, 4 >( right_imaginary, right_scalar )`:
  - Imaginary vector expression evaluates `left_imaginary * right_scalar + right_imaginary * left_scalar + cross( left_imaginary, right_imaginary )`, where `cross` uses `rotated( 1 )` and `rotated( 2 )`.
  - Real scalar expression evaluates `left_scalar * right_scalar - dot( left_imaginary, right_imaginary )`, where `dot` uses `sum( left_imaginary * right_imaginary )`.
  - Reassembly calls `point`'s range-plus-trailing-scalar constructor `point< float, 4 >( imaginary_part, scalar_part )`.

### Symmetric Quaternion Vector Rotation `quaternion_rotate`

- Rodrigues-Gibbs double-cross identity for unit quaternion `rotation = point< float, 4 >( imaginary_part, scalar_part )` acting on a 3D `vector`:
  - `twice_cross = ( cross( rotation_imaginary, vector ) * 2.0f ) | to`
  - `rotated_vector = ( vector + twice_cross * rotation_scalar + cross( rotation_imaginary, twice_cross ) ) | to`
- Evaluates strictly through element-wise range operators and cyclic `cross` permutations across all three dimensions simultaneously.

### Symmetric `orientation_basis` Generation

- Orthonormal basis columns are the rotated images of the canonical unit axes generated from `point< float, 3 >{ 1.0f, 0.0f, 0.0f }` via cyclic `rotated` views with offsets `{ 0, 2, 1 }`:
  - Offset `0` yields `( 1.0f, 0.0f, 0.0f )`, mapping through `quaternion_rotate( rotation )` to the local `right` column.
  - Offset `2` yields `( 0.0f, 1.0f, 0.0f )`, mapping through `quaternion_rotate( rotation )` to the local `up` column.
  - Offset `1` yields `( 0.0f, 0.0f, 1.0f )`, mapping through `quaternion_rotate( rotation )` to the local `forward` column.
- Piping those three rotated views through `quaternion_rotate( rotation )` and materializing with `| to` produces a flat `array< point< float, 3 >, 3 >` basis in one declarative pipeline:

```cpp
using	quaternion	=	point< float, 4 >;

struct __imaginary_part : range_adaptor_closure< __imaginary_part >
{
	constexpr auto operator ( ) ( const quaternion& rotation ) const noexcept -> point< float, 3 >
	{ return point< float, 3 >( rotation ); }
};
inline constexpr auto imaginary_part = __imaginary_part{ };

struct __scalar_part : range_adaptor_closure< __scalar_part >
{
	constexpr auto operator ( ) ( const quaternion& rotation ) const noexcept -> float
	{ return rotation[ 3 ]; }
};
inline constexpr auto scalar_part = __scalar_part{ };

struct __hamilton_product
{
	constexpr auto operator ( ) ( const quaternion& left, const quaternion& right ) const noexcept -> quaternion
	{
		const point< float, 3 > left_imaginary = imaginary_part( left );
		const point< float, 3 > right_imaginary = imaginary_part( right );
		const float left_scalar = scalar_part( left );
		const float right_scalar = scalar_part( right );
		return	point< float, 4 >(
			 left_imaginary * right_scalar + right_imaginary * left_scalar + cross( left_imaginary, right_imaginary )
			,left_scalar * right_scalar - dot( left_imaginary, right_imaginary )
		);
	}
};
inline constexpr auto hamilton_product = __hamilton_product{ };

struct __quaternion_rotate
{
	struct closure : range_adaptor_closure< closure >
	{
		quaternion m_rotation;
		constexpr explicit closure( const quaternion& rotation ) : m_rotation( rotation ) { }
		template< viewable_range t_vector >
		constexpr auto operator ( ) ( t_vector&& vector ) const noexcept -> point< float, 3 >
		{ return __quaternion_rotate{ }( m_rotation, ::std::forward< t_vector >( vector ) ); }
	};

	template< viewable_range t_vector >
	constexpr auto operator ( ) ( const quaternion& rotation, t_vector&& vector ) const noexcept -> point< float, 3 >
	{
		const point< float, 3 > rotation_imaginary = imaginary_part( rotation );
		const float rotation_scalar = scalar_part( rotation );
		const point< float, 3 > twice_cross = ( cross( rotation_imaginary, vector ) * 2.0f ) | to;
		return	( vector + twice_cross * rotation_scalar + cross( rotation_imaginary, twice_cross ) ) | to;
	}

	constexpr auto operator ( ) ( const quaternion& rotation ) const noexcept -> closure
	{ return closure{ rotation }; }
};
inline constexpr auto quaternion_rotate = __quaternion_rotate{ };

struct __orientation_basis
{
	constexpr auto operator ( ) ( const quaternion& rotation ) const noexcept -> array< point< float, 3 >, 3 >
	{
		constexpr point< float, 3 > basis_seed{ 1.0f, 0.0f, 0.0f };
		constexpr array< size_t, 3 > basis_offsets{ 0, 2, 1 };
		return	basis_offsets
			| transform( bind_front( rotated, basis_seed ) )
			| transform( quaternion_rotate( rotation ) )
			| to;
	}
};
inline constexpr auto orientation_basis = __orientation_basis{ };
```

### Normalization Invariant

- Existing `sak::math::normalize` (`vector / length( vector )` in `include/sak/math/vector.hpp`) operates over arbitrary-dimension ranges and normalizes 4D `point< float, 4 >` quaternions directly via `rotation | normalize | to`.
- Re-normalizing accumulated incremental rotations prior to `orientation_basis` preserves unit quaternion norm, keeping the generated basis strictly orthonormal (`determinant == +1.0f`).

## CPU -> GPU Transfer & Local-Frame Ray Transform

### Flat SSBO Representation

- CPU figure base payload stores three consecutive `point< float, 3 >` basis columns (`9` contiguous floats totaling `36` bytes, matching `camera` in [Baseline Reference](baseline.md)) populated from `orientation_basis( m_orientation )` before buffer flush.
- GLSL unpacks the three columns directly into a column-major `mat3` inside `ssbo_figure` using `vec3_from`:
  - `mat3 orientation = mat3( vec3_from( figure_ssbo.data, base_index + orientation_offset ), vec3_from( figure_ssbo.data, base_index + orientation_offset + 3u ), vec3_from( figure_ssbo.data, base_index + orientation_offset + 6u ) );`

### Local-Frame Ray Transformation

- Prior to invoking `evaluate_<figure>`, the outer GPU dispatch loop transforms the world ray into `figure_item`'s local canonical coordinate frame via the orthonormal inverse `transpose( figure_item.orientation )`:

```glsl
//	orthonormal world-to-local ray transformation in the outer dispatch loop
mat3	world_to_local = transpose( figure_item.orientation );
vec3	local_origin = world_to_local * ( ray_origin - figure_item.position );
vec3	local_direction = world_to_local * unit_direction;
```

- Rotational invariance properties under `determinant == +1.0`:
  - Unit ray length holds identically (`length( local_direction ) == 1.0`), supplying `local_direction` directly as the local evaluator's `unit_direction`.
  - World-metric surface depth `hit_distance` remains invariant under orthonormal rotation.
  - Surface-ray inclination cosine `cos_incidence = abs( dot( local_direction, local_normal ) )` remains invariant under orthonormal rotation.
  - Every concrete figure evaluator executes in canonical local coordinates with zero orientation code inside per-figure shader bodies:
    - Analytical sphere and axis-aligned box ([Agnostic Pipeline](agnostic-pipeline.md)).
    - Convex rounded box and cut-plane concave cavity ([Rounded Primitives](rounded-primitives.md)).
    - Adaptive triangle mesh ([Adaptive Mesh](adaptive-mesh.md)).

## Open Questions

| Topic | Option A | Option B | Trade-Off |
|---|---|---|---|
| **C++ SSBO field declaration** | Single `array< point< float, 3 >, 3 > m_orientation` member assigned directly from `orientation_basis` | Three named `point< float, 3 >` members (`m_right`, `m_up`, `m_forward`) mirroring `camera` | Both compile to identical 9-float `is_flat` memory; Option A assigns in one statement, Option B names each basis axis explicitly |
| **Axis-angle quaternion constructor** | Dedicated `axis_angle_quaternion( axis, angle )` niebloid in `sak::math` | Inline construction via `point< float, 4 >( axis * sine( half_angle ), cosine( half_angle ) )` | Option A centralizes the half-angle conversion beside `hamilton_product` for scene and test ergonomics |
| **GLSL `mat3` unpacking helper** | Dedicated `#define mat3_from( data, index )` macro alongside `vec3_from` | Inline `mat3( vec3_from( ... ), vec3_from( ... ), vec3_from( ... ) )` inside `ssbo_figure` | Option A matches the baseline `vec*_from` macro family when multiple structs carry `mat3` fields |

## Boundary

- Base figure payload contract (`figure_payload` with `m_parameters`) and shader dispatch loop belong to [Agnostic Pipeline](agnostic-pipeline.md).
- Local-space primitive intersection formulas belong to [Rounded Primitives](rounded-primitives.md).
- Local-space triangle mesh intersection and crease fillet formulas belong to [Adaptive Mesh](adaptive-mesh.md).
- Orientation updates apply per frame on the CPU; temporal keyframe animation and spherical linear interpolation belong to higher-level scene controllers outside this domain.
- Visible-object discovery and spatial neighborhood links belong to [Spatial Graph](spatial-graph.md).

## Relations

- Parent domain: [3D Renderer](index.md)
- Upstream 4D coordinate storage and vector niebloids: [Point Niebloid Purification](point-niebloids.md)
- Upstream base figure payload and dispatch loop: [Agnostic Pipeline](agnostic-pipeline.md)
- Rotated analytical primitive evaluators: [Rounded Primitives](rounded-primitives.md)
- Rotated adaptive mesh evaluators: [Adaptive Mesh](adaptive-mesh.md)
- Orthogonal per-object visibility indexing: [Spatial Graph](spatial-graph.md)
- Downstream articulated submesh placement consumer: [Legacy Game Preservation](pickup-express.md)
