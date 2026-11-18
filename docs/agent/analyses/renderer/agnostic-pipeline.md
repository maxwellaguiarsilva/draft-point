# Agnostic Pipeline

## Scope

- Decouple CPU scene description from concrete primitive geometry while generating the GLSL figure evaluation and dispatch shader on the CPU:
  - Replace the single-type `sphere_ssbo` loop of [Baseline Reference](baseline.md) with a unified loop over a fixed-stride heterogeneous flat figure SSBO.
  - Assemble fragment shader evaluation functions and type dispatch on the CPU via named token injection into a standalone fragment template under `data/shader/`, compiled through `sak::opengl::shader( source, type )`.
  - Upgrade ray traversal to per-fragment normalized `unit_direction` and world-metric surface entry distance `hit_distance` across curved (`sphere`, `capsule`) and planar (3D `triangle`) primitives.
  - Planned zero-waste revision: [Per-Kind Figure Buffers](per-kind-figure-buffers.md).

## Architectural Layers

- **Layer 1 — CPU Figure Contract and Token-Injected Shader Assembly**:
  - Fixed-stride `is_flat` figure payload contract with compile-time figure trait registration and a single slot-capacity `static_assert`.
  - Shared GLSL dimension-symmetry prelude combined with three named placeholder tokens and `#line` diagnostic restoration.
  - In-memory shader compilation via `sak::opengl::shader( source, type )` and program linking via `sak::opengl::program`, keeping `sak::opengl::shader::loader` and baseline files unchanged.
- **Layer 2 — Heterogeneous Sphere, Trigonometric 3D Triangle, and Capsule Dispatch**:
  - Single `storage_buffer` populated with heterogeneous sphere, CPU-precomputed 3D plane triangle, and capsule payloads in a standalone prototype under `tests/adhoc/rendering/`.
  - CPU `triangle` class storing absolute `triangle::data` vertices while precomputing the triangle centroid, three centroid-relative vertex vectors, and plane unit normal for flat SSBO delivery.
  - CPU `capsule` class storing two absolute world endpoints (`capsule::data = array< position, 2 >`) and `m_radius` while deriving midpoint `position( )`, unit axis, and `half_length` for flat SSBO delivery.
  - Per-fragment `unit_direction` normalization prior to figure iteration.
  - Analytical ray-surface intersection across sphere, trigonometric 3D plane triangle (with sub-triangle oriented-area membership), and capsule (finite cylinder body and two hemispherical endcaps), paired with unified inclination shading (`ambient + volume * cos_incidence`) and world-metric `hit_distance` depth sorting across curved and planar surfaces.

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
| `m_position` | `sak::point< float, 3 >` | `0` | `3` | World-space center of the figure (sphere center, precomputed triangle centroid, or capsule midpoint) |
| `m_type_id` | `float` | `3` | `1` | Exact integer-valued `float` (`0.0f`, `1.0f`, or `2.0f`), decoded in GLSL via `uint( figure_item.type_id )` |
| `m_color` | `sak::point< float, 4 >` | `4` | `4` | Flat RGBA base color |
| `m_parameters` | `std::array< float, t_parameter_count >` | `8` | `t_parameter_count` | Fixed-capacity figure parameter slots (`1` float for sphere radius; `12` floats for triangle centroid-relative vectors and plane unit normal; `5` floats for capsule unit axis, `half_length`, and `radius`); registration validates that every figure fits within `t_parameter_count` floats |

- Exact integer-valued `float` encoding for `m_type_id`:
  - `m_type_id` stores an exact integer-valued `float` (`0.0f`, `1.0f`, `2.0f`, contiguous across integers up to $2^{24} = 16{,}777{,}216$) and converts on the GPU via `uint( figure_item.type_id )`.
  - Conforms strictly to [SSBO Reflection](../../ssbo-reflection.md) ("Only float-based payload members" over `float data[ ]`).
  - Exact integer-valued `float` representation preserves normal IEEE-754 exponent encoding across `float data[ ]` transfers and guarantees lossless `uint( figure_item.type_id )` conversion across IEEE-754 GPU hardware.
- Extensibility to variable-size figures ([Adaptive Mesh](adaptive-mesh.md)):
  - Multi-triangle meshes preserve the fixed stride of `figure_ssbo` by storing secondary SSBO descriptor scalars inside `m_parameters` that index flat buffers at higher bindings:
    - `first_triangle`
    - `triangle_count`
    - `bounding_radius`

### 2. Unified Intersection and Inclination Signature

- Per-fragment normalization:
  - `vec3 unit_direction = normalize( direction );` is computed once per fragment prior to iterating `figure_ssbo.count`.
- Evaluator signature:
  - Every registered figure emits `hit_result evaluate_<figure>( vec3 ray_origin, vec3 unit_direction, figure figure_item )`.

aguardando o usuário finalizar o modelo final do estágio dois para documentar após estar concluído

- Mathematical properties of the sphere, 3D plane triangle, and capsule evaluators:
  - Sphere (`parameters[ 0 ] = radius`): `near_distance = along - radius * cos_incidence` provides exact world-space exterior entry depth and `far_distance = along + radius * cos_incidence` provides interior exit depth for `hit_distance`.
  - 3D Plane Triangle (`position = centroid`, `parameters[ 0..8 ] = three centroid-relative vectors`, `parameters[ 9..11 ] = plane unit normal`):
    - CPU `triangle` class owns absolute vertex coordinates in `triangle::data` (`array< position, 3 >`) and precomputes the centroid, three centroid-relative vectors (`vertex - centroid`), and unit plane normal `normalize( cross( v1 - v0, v2 - v0 ) )`.
    - Trigonometric ray-plane intersection projects the camera-to-centroid offset onto the plane unit normal (`dot( centroid - ray_origin, normal )`) and divides by `dot( unit_direction, normal ) = cos( theta )` to obtain the exact world-metric `hit_distance` along `unit_direction`.
    - Sub-triangle oriented-area membership tests the centroid-relative hit point against the three centroid-relative vertex vectors (`dot( cross( corner_a, corner_b ), normal )`).
    - Surface inclination reuses the intersection cosine directly via `cos_incidence = abs( dot( unit_direction, normal ) )`, shading both faces identically by camera-ray inclination.
  - Capsule (`position = midpoint`, `parameters[ 0..2 ] = unit_axis`, `parameters[ 3 ] = half_length`, `parameters[ 4 ] = radius`):
    - CPU `capsule` class owns two absolute world endpoints in `capsule::data` (`array< position, 2 >`) and `m_radius`, and derives midpoint `position( )`, unit axis (`normalize( endpoint_1 - endpoint_0 )`), and `half_length` (`0.5f * length( endpoint_1 - endpoint_0 )`) for SSBO delivery.

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
  - The shared payload capacity is declared once as `parameter_capacity` (`12uz`), which fixes `t_parameter_count` of the figure payload.
  - A single `static_assert` over the registered tuple rejects any trait whose `m_parameter_count` exceeds that capacity.
- Generated dispatch statement:
  - The CPU emits `switch( uint( figure_item.type_id ) )` with one `case` per registered trait.
  - `m_type_id` remains an exact integer-valued `float` inside the payload and converts on the GPU via `uint( figure_item.type_id )`.
- Registered traits in the Stage 2 prototype:
  - `sphere_figure`: `m_type_id = 0u`, `m_parameter_count = 1uz`, evaluator `evaluate_sphere`.
  - `triangle_figure`: `m_type_id = 1u`, `m_parameter_count = 12uz`, evaluator `evaluate_triangle`.
  - `capsule_figure`: `m_type_id = 2u`, `m_parameter_count = 5uz`, evaluator `evaluate_capsule`.

```cpp
inline constexpr tuple figure_traits{ sphere_figure{ }, triangle_figure{ }, capsule_figure{ } };

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
  - Restoration line numbers are computed relative to the fragment section, whose first line is `#version 460 core`.

## Resolved Decisions

| Decision Area | Decision | Rationale |
|---|---|---|
| C++ figure registration model | Compile-time figure trait types in a `constexpr std::tuple` | Guarantees payload-slot and GLSL parameter alignment at compile time with no runtime descriptor table |
| CPU triangle precomputation | Dedicated `triangle` class storing absolute `triangle::data` vertices and precomputing centroid, three centroid-relative vectors, and unit plane normal | Isolates vertex ownership in `triangle::data` and reduces GPU per-fragment triangle evaluation to ray-plane intersection, sub-triangle oriented areas, and inclination dot product |
| CPU capsule precomputation | Dedicated `capsule` class storing absolute `capsule::data` endpoints (`array< position, 2 >`) and `m_radius` while deriving midpoint `position( )`, unit axis, and `half_length` | Isolates endpoint ownership in `capsule::data` and reduces GPU per-fragment capsule evaluation to analytical cylinder-body and hemispherical-endcap intersection |
| Generated GLSL dispatch statement | `switch( uint( figure_item.type_id ) )` | Expresses integer table dispatch directly in GLSL 460, and the boilerplate is generated by the CPU rather than written per figure |
| Fragment template storage | Standalone `.glsl` template under `data/shader/` read through `sak::fso::text_file` | Separates shader authoring from C++ and keeps `sak::opengl::shader::loader` file-based and unmodified |

### Implementation Reference

- Stage 2 prototype: `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`.
- Fragment template: `data/shader/agnostic-pipeline.glsl`.
- The prototype exercises the heterogeneous `figure_ssbo` pipeline with a sphere (`sphere_figure` / `evaluate_sphere`), a trigonometric 3D plane triangle (`triangle` + `triangle_figure` / `evaluate_triangle`), and a capsule (`capsule` + `capsule_figure` / `evaluate_capsule`).

## Boundary

- Baseline contracts preserved unchanged:
  - `data/shader/sdl3-shadertoy.glsl`
  - `sak::opengl::shader::loader`
  - `environment_ssbo`
  - `camera_ssbo`
- This node specifies sphere, 3D plane triangle, and capsule evaluators; convex `border_radius` rounding and concave interior fillets belong to [Rounded Primitives](rounded-primitives.md).
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
