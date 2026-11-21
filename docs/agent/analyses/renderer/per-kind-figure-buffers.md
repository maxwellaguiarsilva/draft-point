# Per-Kind Figure Buffers

## Scope

- Planned revision of [Agnostic Pipeline](agnostic-pipeline.md): replace the single fixed-stride heterogeneous figure buffer and `type_id` switch with one homogeneous buffer per figure kind.
- The payload of each kind carries only the fields its evaluator reads, so the stride equals the used size and no slot is zero-filled.
- This document records the target contract and the change surface. It is a planning document owned by the user; the open decisions remain editable until the model is finalized.

## Motivation

- The shared payload reserves twelve parameter floats for every kind, and a type id, regardless of how many floats the evaluator reads.
- The same stride is uploaded for every figure in one buffer, so the three kinds in the prototype allocate and transfer more storage than they consume.

| Figure | Used today | Allocated today | Target |
|---|---:|---:|---:|
| `sphere` | 9 | 20 | 8 |
| `triangle` | 20 | 20 | 19 |
| `capsule` | 13 | 20 | 12 |
| **total** | 42 | **60** | **39** |

- The target model removes the unused slots, the type id, and the switch, and makes each buffer homogeneous so the shader loop and the evaluator signature are generated per kind.

## Target Model

- One SSBO per figure kind, homogeneous within the buffer.
- One payload type per kind with named fields, satisfying `is_flat`.
- `stride = sizeof( t_payload ) / sizeof( float )`, exact by construction.
- No `m_type_id` member and no integer dispatch.
- One generated traversal loop per kind, with the nearest-hit accumulation shared.
- Every binding, offset, stride, and evaluator association derived by the machine from declarations, never authored as a number.

## Identity and Binding Contract

- The programmer declares a payload type, its evaluator, and its entry in the payload registry. The programmer authors no index, offset, stride, or type id.
- The machine derives from the registry:
  - Binding index: the position of the payload type in the ordered registry, as defined by [SSBO Reflection](../../ssbo-reflection.md).
  - GLSL block binding literal: the same registry position, emitted during shader assembly.
  - Member float offsets: `offset_of( member ).bytes / sizeof( float )`.
  - Element stride: `sizeof( t_payload ) / sizeof( float )`.
  - Evaluator association: the registry entry itself, so the traversal loop calls the evaluator of that kind with no lookup table.
- Identity is therefore a consequence of the declaration order, not a value maintained by hand.

```cpp
//	one ordered payload registry; every index below is derived from the position
inline constexpr tuple payload_registry{
	 type_identity< environment >{ }
	,type_identity< camera >{ }
	,type_identity< figure_payload< figure_trait::sphere::parameters > >{ }
	,type_identity< figure_payload< figure_trait::triangle::parameters > >{ }
	,type_identity< figure_payload< figure_trait::capsule::parameters > >{ }
};

//	binding of a payload type is its registry position
template< typename t_payload >
consteval auto binding_of( ) -> GLuint
{
	return	static_cast< GLuint >( *index_of( payload_registry, type_identity< t_payload >{ } ) );
}
```

- Adding or removing a figure changes the registry, and every binding, emitted literal, and traversal loop follows automatically.
- Adding a figure is a declaration, not an increment: there is no counter to advance and no identifier to choose.
- `environment` and `camera` are ordinary registry entries, so the figure bindings continue after them without a separate counter.
- Validation belongs to the machine as well:
  - The registry rejects a duplicated payload type.
  - A payload absent from the registry fails a `static_assert` where its binding is derived.
  - The registry size is checked against the shader storage binding limit of the target context.

## Change Surface

### 1. Payload and Parameters

- Replace the capacity-based payload with a common prefix plus a typed parameter tail:

```cpp
template< typename t_parameters >
struct figure_payload
{
	position	m_position;
	color		m_color;
	t_parameters	m_parameters;
};
```

- Each trait owns its parameter type and no numeric identity:

```cpp
struct figure_trait
{
	struct sphere
	{
		struct parameters { float m_radius; };
	};

	struct triangle
	{
		struct parameters
		{
			array< position, 3 >	m_corner;
			direction		m_normal;
		};
	};

	struct capsule
	{
		struct parameters
		{
			direction	m_axis;
			float		m_half_length;
			float		m_radius;
		};
	};
};
```

- `is_figure_trait` requires the nested `parameters` type and the evaluator name; it no longer requires a parameter count or a type id.
- `parameter_capacity` and the `payload_within_capacity` validation are removed; `is_flat` on the concrete payload replaces the capacity check.

### 2. Figure Traits and Sources

- `evaluator_identifier` remains the link between a trait and its `.glsl` file.
- The source loader keeps only the evaluator text; it no longer assigns a type id.
- `ensure_figure_sources` still asserts that every registered trait has a loaded evaluator.

### 3. Buffers

- Replace the single `storage_buffer< figure >` with one `storage_buffer< payload >` per figure kind.
- `sak::opengl::shader::storage_buffer` keeps its contract: header `{ count, stride }`, flat float payload, explicit bind.
- Binding uses the derived index:

```cpp
m_spheres.bind( binding_of< sphere_payload >( ) );
```

- The fixed-size constructor needs the per-kind count before allocation. The change selects one of:
  - a counting pass over the scene description;
  - a per-kind `vector< payload >` that the buffer copies at construction;
  - a growable buffer that reallocates and rewrites the header.
- A generated owner derived from `figure_traits` avoids repeating one member, one bind, and one flush per kind.

### 4. GLSL Template

- Remove the `@figure_parameter_capacity` placeholder, `struct figure`, and `ssbo_figure`.
- Generate one block, one value struct, and one accessor per kind, with the binding and offsets derived as described above:

```glsl
layout( std430, binding = <derived> ) readonly buffer sphere_buffer
{ uint count; uint stride; float data[]; } sphere_ssbo;

struct sphere
{
	vec3	position;
	vec4	color;
	float	radius;
};

sphere sphere_at( uint index )
{
	uint base_index = index * sphere_ssbo.stride;
	return	sphere(
		 vec3_from( sphere_ssbo.data, base_index + 0u )
		,vec4_from( sphere_ssbo.data, base_index + 3u )
		,sphere_ssbo.data[ base_index + 7u ]
	);
}
```

- Remove `dispatch_figure` and the `switch`.
- Factor the nearest-hit update into one helper:

```glsl
void accumulate( inout float best_distance, inout vec3 color, hit_result candidate, vec4 item_color, environment env )
```

- In `main`, emit one loop per figure registry entry, each reading its own buffer and calling its own evaluator through `accumulate`.

### 5. Evaluators

| File | Current access | Target access |
|---|---|---|
| `data/shader/figures/sphere.glsl` | `figure`, `parameters[ 0 ]` | `sphere`, `item.m_radius` |
| `data/shader/figures/triangle.glsl` | `parameters[ 0..8 ]`, `parameters[ 9..11 ]` | `item.m_corner`, `item.m_normal` |
| `data/shader/figures/capsule.glsl` | `parameters[ 0..2 ]`, `parameters[ 3 ]`, `parameters[ 4 ]` | `item.m_axis`, `item.m_half_length`, `item.m_radius` |

- Each evaluator signature takes its own payload value type.
- The field names follow the C++ declaration; the generator removes the `m_` prefix as recorded in [SSBO Reflection](../../ssbo-reflection.md).

### 6. CPU Shapes and Registration

- `triangle::parameters( )` and `capsule::parameters( )` return the trait parameter type instead of a fixed array.
- `register_figure` produces the exact payload of one kind and appends it to that kind's storage, replacing the shared out-parameter and the zero fill.
- The sphere registration passes `{ m_radius }` directly instead of a one-element array.

### 7. Prototype and Placeholders

- `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`:
  - drops `parameter_capacity`, `payload_within_capacity`, `m_type_id`, `figure_case`, and `figure_dispatch`;
  - keeps `figure_loader` only as the source-text loader;
  - generates buffers, accessors, and traversal loops from `figure_traits`.
- `glsl_placeholders` changes to the generated blocks:
  - buffer and accessor declarations per kind;
  - evaluator definitions;
  - traversal loops per kind.

## Layout Constraint

- The SSBO stays a `float data[]` array read through generated offsets.
- Declaring the block with `vec3` or `vec4` members would let `std430` insert alignment padding and break the exact stride, as recorded in [SSBO Reflection](../../ssbo-reflection.md).
- The GLSL value struct is an in-memory reconstruction, not the block layout; `vec3_from` and `vec4_from` continue to rebuild vectors from consecutive floats.

## Trade-Offs

| Gain | Cost |
|---|---|
| Exact stride and no zero-filled slots | One GLSL block and one loop generated per kind |
| No type id and no switch | One bind and one flush per kind |
| Payload field names used directly by evaluators | Per-kind count needed before buffer allocation |
| Indices derived from the registry | More bindings occupied |

## Open Decisions

- Registry composition: one flat tuple that lists every payload, or a tuple derived from `figure_traits` and concatenated after `environment` and `camera`.
- Buffer ownership: a generated tuple of `storage_buffer` members, a `map` keyed by trait, or a dedicated figure-buffer owner type.
- Header content: keep `{ count, stride }` uniformly, or drop the per-buffer stride because each generated accessor knows its exact field count.
- Accessor location: generated from reflected C++ payloads, or authored next to each evaluator.
- Count acquisition: counting pass, per-kind vector, or a growable buffer.
- Runtime validation: query the context binding limit and reject a registry that exceeds it.

## Relations

- Parent domain: [3D Renderer](index.md)
- Revised model: [Agnostic Pipeline](agnostic-pipeline.md)
- Governing contracts:
  - [SSBO Reflection](../../ssbo-reflection.md) — payload registry, derived bindings, generated accessors, flat-float layout
- Implementation reference: `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`, `data/shader/agnostic-pipeline.glsl`, `data/shader/figures/*.glsl`
