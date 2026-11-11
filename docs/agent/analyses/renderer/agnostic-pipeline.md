# Agnostic Pipeline

## Scope

- Decouple CPU scene description from concrete primitive geometry while generating the GLSL figure evaluation and dispatch shader on the CPU:
  - Replace the single-type `sphere_ssbo` loop of [Baseline Reference](baseline.md) with a unified loop over a fixed-stride heterogeneous flat figure SSBO.
  - Assemble fragment shader evaluation functions and type dispatch on the CPU via named token injection into a standalone fragment template under `data/shader/`, compiled through `sak::opengl::shader( source, type )`.
  - Upgrade ray traversal to per-fragment normalized `unit_direction` and world-metric surface entry distance `hit_distance` across curved and flat primitives.

## Architectural Layers

- **Layer 1 — CPU Figure Contract and Token-Injected Shader Assembly**:
  - Fixed-stride `is_flat` figure payload contract with compile-time figure trait registration and a single slot-capacity `static_assert`.
  - Shared GLSL dimension-symmetry prelude combined with three named placeholder tokens and `#line` diagnostic restoration.
  - In-memory shader compilation via `sak::opengl::shader( source, type )` and program linking via `sak::opengl::program`, keeping `sak::opengl::shader::loader` and baseline files unchanged.
- **Layer 2 — Heterogeneous Sphere and Slab-Cube Dispatch**:
  - Single `storage_buffer` populated with heterogeneous sphere (`x = -0.55f`) and axis-aligned slab-cube (`x = +0.55f`, `y = 0.0f`, `z = 1.0f`) payloads in a standalone prototype under `tests/adhoc/rendering/`.
  - Per-fragment `unit_direction` normalization prior to figure iteration.
  - Unified inclination shading (`ambient + volume * cos_incidence`) and world-metric `hit_distance` depth sorting across curved and flat surfaces.

## Verified Infrastructure Facts

| Capability | Codebase Mechanism | Evidence |
|---|---|---|
| In-memory GLSL compilation | `sak::opengl::shader( const string& source, const type shader_type = vertex )` compiles CPU-assembled `std::string` sources directly | `include/sak/opengl/shader.hpp` (`shader::shader`) |
| Arbitrary shader-range linking | `sak::opengl::program< t_range >` links any `is_shader_range` (such as `std::array< shader, 2 >`), keeping file-based `shader::loader` untouched | `include/sak/opengl/program.hpp` (`program::program`), `include/sak/opengl/shader.hpp` (`is_shader_range`) |
| Flat SSBO layout enforcement | `sak::opengl::shader::storage_buffer< t_payload >` static-asserts `is_flat< t_payload >` and prefixes `float data[ ];` with `{ count, stride }` in float units | `include/sak/opengl/shader.hpp` (`has_no_padding`, `is_flat`, `storage_buffer`), [SSBO Reflection](../../ssbo-reflection.md) |
| String formatting boundary | `include/sak/string.hpp` provides `to_lower_case` and `to_upper_case` only, while `include/sak/format.hpp` wraps `std::vformat` which parses `{` and `}` as format specifiers | `include/sak/string.hpp`, `include/sak/format.hpp` |

## Premeditated Technical Contracts

### 1. Flat Heterogeneous Payload and Type-ID Encoding

```cpp
//	fixed-stride flat figure payload shared across all registered figure types
template< size_t t_parameter_count >
struct figure_payload
{
	point< float, 3 >					m_position;
	float								m_type_id;
	point< float, 4 >					m_color;
	array< float, t_parameter_count >	m_parameters;
};
```

| Field | C++ Type | Float Offset | Float Count | Contract |
|---|---|---|---|---|
| `m_position` | `sak::point< float, 3 >` | `0` | `3` | World-space center of the figure |
| `m_type_id` | `float` | `3` | `1` | Exact integer-valued `float` (`0.0f` or `1.0f`), decoded in GLSL via `uint( figure_item.type_id )` |
| `m_color` | `sak::point< float, 4 >` | `4` | `4` | Flat RGBA base color |
| `m_parameters` | `std::array< float, t_parameter_count >` (or `sak::point< float, t_parameter_count >` when `t_parameter_count > 1`) | `8` | `t_parameter_count` | Fixed-capacity figure parameter slots; registration validates that every figure fits within `t_parameter_count` floats |

- Exact integer-valued `float` encoding for `m_type_id`:
  - `m_type_id` stores an exact integer-valued `float` (`0.0f`, `1.0f`, contiguous across integers up to $2^{24} = 16{,}777{,}216$) and converts on the GPU via `uint( figure_item.type_id )`.
  - Conforms strictly to [SSBO Reflection](../../ssbo-reflection.md) ("Only float-based payload members" over `float data[ ]`).
  - Exact integer-valued `float` representation preserves normal IEEE-754 exponent encoding across `float data[ ]` transfers and guarantees lossless `uint( figure_item.type_id )` conversion across IEEE-754 GPU hardware.
- Parameter-slot container choice:
  - `std::array< float, t_parameter_count >` supports any slot capacity `t_parameter_count >= 1`, while `sak::point< float, t_parameter_count >` supports `t_parameter_count > 1` (`include/sak/geometry/point.hpp`).
- Extensibility to variable-size figures ([Adaptive Mesh](adaptive-mesh.md)):
  - Multi-triangle meshes preserve the fixed stride of `figure_ssbo` by storing secondary SSBO descriptor scalars inside `m_parameters` that index flat buffers at higher bindings:
    - `first_triangle`
    - `triangle_count`
    - `bounding_radius`

### 2. Unified Intersection and Inclination Signature

- Per-fragment normalization:
  - `vec3 unit_direction = normalize( direction );` is computed once per fragment prior to iterating `figure_ssbo.count`.
- Evaluator signature and outer loop:
  - Every registered figure emits `hit_result evaluate_<figure>( vec3 ray_origin, vec3 unit_direction, figure figure_item )`.
  - The outer loop dispatches each candidate via `dispatch_figure( camera_state.position, unit_direction, figure_item )`:
    - Accepts the closest front hit satisfying `candidate.hit` and `candidate.hit_distance > 0.0` and `candidate.hit_distance < best_distance`.
    - Shades the winning fragment as `figure_item.color.rgb * ( environment_state.ambient + environment_state.volume * candidate.cos_incidence )`.

```glsl
//	unified surface hit contract returned by every injected figure evaluator
struct hit_result
{
	bool	hit;
	float	hit_distance;
	float	cos_incidence;
};

//	unit-ray sphere evaluator returning world-metric hit distance and inclination cosine
hit_result evaluate_sphere( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	float	radius = figure_item.parameters[ 0 ];
	vec3	local_center = figure_item.position - ray_origin;
	float	along = dot( local_center, unit_direction );
	float	opposite_leg_squared = dot( local_center, local_center ) - square( along );
	float	radius_squared = square( radius );
	if( opposite_leg_squared > radius_squared )
		return	hit_result( false, -1.0, 0.0 );
	float	ratio = opposite_leg_squared / radius_squared;
	float	cos_incidence = sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) );
	float	half_chord = radius * cos_incidence;
	float	near_distance = along - half_chord;
	float	far_distance = along + half_chord;
	if( far_distance <= 0.0 )
		return	hit_result( false, -1.0, 0.0 );
	float	hit_distance = near_distance > 0.0 ? near_distance : far_distance;
	return	hit_result( true, hit_distance, cos_incidence );
}

//	dimension-symmetric axis-aligned slab box evaluator
hit_result evaluate_cube( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	vec3	half_size = vec3( figure_item.parameters[ 0 ] );
	vec3	local_origin = ray_origin - figure_item.position;
	vec3	inverse_direction = 1.0 / unit_direction;
	vec3	first_slab = ( -half_size - local_origin ) * inverse_direction;
	vec3	second_slab = ( half_size - local_origin ) * inverse_direction;
	vec3	near_slab = min( first_slab, second_slab );
	vec3	far_slab = max( first_slab, second_slab );
	float	enter_distance = max_axis( near_slab );
	float	exit_distance = min_axis( far_slab );
	if( exit_distance < max( enter_distance, 0.0 ) )
		return	hit_result( false, -1.0, 0.0 );
	if( exit_distance <= 0.0 )
		return	hit_result( false, -1.0, 0.0 );
	bool	is_exterior = enter_distance > 0.0;
	float	hit_distance = is_exterior ? enter_distance : exit_distance;
	vec3	face_mask = is_exterior ? step( vec3( enter_distance ), near_slab ) : step( far_slab, vec3( exit_distance ) );
	float	cos_incidence = dot( abs( unit_direction ), face_mask ) / length( face_mask );
	return	hit_result( true, hit_distance, cos_incidence );
}
```

- Mathematical properties of the sphere and slab-cube evaluators:
  - Sphere (`parameters[ 0 ] = radius`): `cos_incidence = sqrt( 1.0 - ratio )` matches [Baseline Reference](baseline.md) identically while `near_distance = along - radius * cos_incidence` provides exact world-space exterior entry depth and `far_distance = along + radius * cos_incidence` provides interior exit depth for `hit_distance`.
  - Axis-aligned Cube (`parameters[ 0 ] = half_extent` or `vec3 half_extents`): `face_mask = step( vec3( enter_distance ), near_slab )` isolates the active entry slab across all three axes simultaneously, and `dot( abs( unit_direction ), face_mask ) / length( face_mask )` evaluates `abs( dot( unit_direction, face_normal ) )` across face interiors and edge ties alike.
  - Perspective face separation from camera-ray inclination (`half_extent = 0.45f` at `{ +0.55f, 0.0f, 1.0f }` viewed from `{ 0.0f, 0.0f, -2.0f }`):
    - Front `-z` face yields `cos_incidence` near `0.985` (final shaded brightness `0.3 + 0.7 * 0.985` near `0.99`).
    - Visible inner `-x` face at `x = +0.10` yields `cos_incidence` near `0.033` (final shaded brightness `0.3 + 0.7 * 0.033` near `0.323`), cleanly separating the two faces in 3D from ray-surface inclination alone.
  - Axis-aligned parallel-ray guard: when the camera lies on a slab boundary plane (`abs( local_origin ) == half_size`) along a zero ray component (`0.0 * inf`), clamping denominator magnitude with non-zero sign preservation keeps `inverse_direction` finite across all three axes simultaneously.

### 3. Compile-time Figure Registration Model

- Figures are declared as compile-time trait types collected in one `constexpr std::tuple`, each trait pairing:
  - `m_type_id` (`GLuint` GPU type id).
  - `m_parameter_count` (`size_t` parameter-slot count).
  - `m_evaluator_name` (`std::string_view` evaluator name).
  - `m_evaluator_source` (`std::string_view` static GLSL evaluator definition).
- The trait set is the single source of truth; every generated artifact folds over the same tuple through one per-trait mapping applied with `std::apply`:
  - Evaluator concatenation joins `m_evaluator_source` across every registered trait.
  - Dispatch `case` generation emits one `case <type_id>u: return evaluate_<name>( ray_origin, unit_direction, figure_item );` per registered trait.
  - Slot-capacity validation folds `m_parameter_count <= t_parameter_count` across every registered trait.
- Capacity constraint stated once and validated at compile time for every registered trait:
  - The shared payload capacity is declared once as `parameter_capacity`, which fixes `t_parameter_count` of the figure payload.
  - A single `static_assert` over the registered tuple rejects any trait whose `m_parameter_count` exceeds that capacity.
- Generated dispatch statement:
  - The CPU emits `switch( uint( figure_item.type_id ) )` with one `case` per registered trait.
  - `m_type_id` remains an exact integer-valued `float` inside the payload and converts on the GPU via `uint( figure_item.type_id )`.
- Registered traits in the Stage 2 prototype:
  - `sphere_figure`: `m_type_id = 0u`, `m_parameter_count = 1uz`, evaluator `evaluate_sphere`.
  - `cube_figure`: `m_type_id = 1u`, `m_parameter_count = 1uz`, evaluator `evaluate_cube`.

```cpp
//	single registered figure set consumed by every generated artifact
inline constexpr tuple figure_traits{ sphere_figure{ }, cube_figure{ } };

//	compile-time validation of the shared payload capacity for every registered figure
static_assert(
	 map_figure_traits( payload_within_capacity )
	,"figure parameter count exceeds the shared payload capacity"
);
```

### 4. GLSL Dimension-Symmetry Prelude

```glsl
//	horizontal reductions and cyclic shifts isolated in the fragment template prelude
vec3 rotated_1( vec3 value ) { return value.yzx; }
vec3 rotated_2( vec3 value ) { return value.zxy; }
float min_axis( vec3 value ) { return min( value.x, min( value.y, value.z ) ); }
float max_axis( vec3 value ) { return max( value.x, max( value.y, value.z ) ); }
float sum_axis( vec3 value ) { return dot( value, vec3( 1.0 ) ); }
```

- Defined once in the static fragment shader template header, confining axis-component swizzles to these five one-line helpers while all figure evaluators operate on whole vectors.

### 5. CPU Placeholder Replacement Mechanics

- Three named placeholder tokens in the standalone fragment template, each pairing a marker line with a replacement produced on the CPU:

| Placeholder Marker | CPU Replacement | Purpose |
|---|---|---|
| `//	@figure_parameter_capacity` | `#define figure_parameter_capacity <count>` | Injects the payload slot capacity so it exists once in C++ (`t_parameter_count`) and is not duplicated as a literal array bound in GLSL |
| `//	@figure_functions` | Concatenated `evaluate_<figure>` definitions | Populates the evaluator bodies folded from the registered trait tuple |
| `//	@figure_dispatch` | Generated `switch( uint( figure_item.type_id ) )` with one `case` per registered trait | Replaces per-figure dispatch boilerplate with a single generated statement |

- Placeholder order and producer pairing:
  - Every placeholder is one `glsl_placeholder{ m_marker, m_produce }` entry inside a single `constexpr` array of three entries.
  - Each producer receives the one-based line number of its marker, so replacement runs in array order over one template copy while every producer reads the unmodified template line layout.
- Deterministic string replacement:
  - Token expansion uses literal substring search and replacement (`std::string::find` and `std::string::replace`), preserving unescaped GLSL `{` and `}` braces directly while `sak::format` targets `std::vformat` format strings.
  - Each injected region ends with a `#line <marker_line + 1>` directive so GLSL compiler diagnostics resume on the template line that follows the marker ([SSBO Reflection](../../ssbo-reflection.md)).
  - Restoration line numbers are computed relative to the fragment section, whose first line is `#version 460 core`.

## Resolved Decisions

| Decision Area | Decision | Rationale |
|---|---|---|
| C++ figure registration model | Compile-time figure trait types in a `constexpr std::tuple` | Guarantees payload-slot and GLSL parameter alignment at compile time with no runtime descriptor table |
| Generated GLSL dispatch statement | `switch( uint( figure_item.type_id ) )` | Expresses integer table dispatch directly in GLSL 460, and the boilerplate is generated by the CPU rather than written per figure |
| Fragment template storage | Standalone `.glsl` template under `data/shader/` read through `sak::fso::text_file` | Separates shader authoring from C++ and keeps `sak::opengl::shader::loader` file-based and unmodified |

### Implementation Reference

- Stage 2 prototype: `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`.
- Fragment template: `data/shader/agnostic-pipeline.glsl`.
- Stage 0 baseline remains unchanged: `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl`.
- The prototype builds and renders one red sphere at `x = -0.55f` beside one blue slab cube at `x = +0.55f`, whose visible `-x` side sliver is shaded markedly darker than its front face, confirming the front/side inclination separation.

## Boundary

- Baseline contracts preserved unchanged:
  - `source/sdl3_shadertoy.cpp`
  - `data/shader/sdl3-shadertoy.glsl`
  - `sak::opengl::shader::loader`
  - `environment_ssbo`
  - `camera_ssbo`
- This node specifies sphere and sharp axis-aligned slab-cube evaluators; convex `border_radius` rounding and concave interior fillets belong to [Rounded Primitives](rounded-primitives.md).
- Arbitrary 3D figure rotation belongs to [Orientation](orientation.md).
- Spatial proximity culling belongs to [Spatial Graph](spatial-graph.md).

## Relations

- Parent domain: [3D Renderer](index.md)
- Upstream contracts:
  - [Baseline Reference](baseline.md)
  - [Point Niebloid Purification](point-niebloids.md)
  - [SSBO Reflection](../../ssbo-reflection.md)
- Downstream consumers:
  - [Rounded Primitives](rounded-primitives.md)
  - [Adaptive Mesh](adaptive-mesh.md)
  - [Orientation](orientation.md)
  - [Spatial Graph](spatial-graph.md)
