# Figure Blocks

- Each `.glsl` file in this folder is one figure block: the host inlines its text directly into the generated dispatch switch of [agnostic-pipeline.glsl](../agnostic-pipeline.glsl).
- The host side that loads, extracts and inlines these files lives in [`tests/adhoc/rendering/0011_agnostic_pipeline.cpp`](../../../tests/adhoc/rendering/0011_agnostic_pipeline.cpp) under `figure_block` and `figure_case`.

## File Identity

- The file stem is the figure name and matches the reflected identifier of a registered figure trait (`figure_trait::sphere` pairs with `sphere.glsl`).
- The loader assigns each figure its numeric type id from the sorted file stems, so adding a file shifts the ids of the stems that sort after it.
- The host asserts that every registered trait has a file in this folder before compiling the fragment shader.

## File Content

- The file is exactly one braced block: it starts with `{`, ends with `}` and carries the block body between them.
- The host supplies the dispatcher signature, the surrounding scope and the return statement; the file carries the block only.
- `return` belongs to the dispatcher: the block reaches its closing brace as its only exit, and `dispatch_figure` returns `result` after the generated switch.
- The outcome of the evaluation goes into the `result` variable that `dispatch_figure` declares before the generated switch.

## Dispatcher Scope

- The block runs inside its own `case` of `dispatch_figure`, so these names are already in scope:

| Name | Type | Meaning |
|---|---|---|
| `result` | `hit_result` | Pre-declared outcome, initialized with `no_hit( )` |
| `figure_item` | `figure` | Payload of the current figure: `position`, `type_id`, `color` and `parameters` |
| `direction` | `vec3` | World-space ray direction, normalized by the dispatcher before the figure loop |
| `ray_origin` | `vec3` | World-space camera position |
| `local_center` | `vec3` | `figure_item.position - ray_origin` |
| `along` | `float` | `dot( local_center, direction )` |

- The dispatcher rejects the figure before reaching the switch when `along <= 0.0`, so the block assumes the figure center lies ahead of the camera.
- Locals declared in the block stay scoped to its own case.

## Result Contract

| Field | Type | Meaning |
|---|---|---|
| `hit` | `bool` | `true` when the ray reaches the surface |
| `hit_distance` | `float` | World-metric distance from the camera along `direction`, positive when the surface entry lies ahead of the camera and negative when it lies behind |
| `hit_normal` | `vec3` | World-space unit surface normal at the hit point, supplied already normalized |

- The block writes `result = hit_result( true, hit_distance, hit_normal )` when the ray reaches the surface.
- The block leaves `result` at its shared `no_hit( )` value when the ray misses, so every figure inherits the same miss outcome.
- Shading belongs to the dispatcher: it saves the closest hit during the traversal and applies `best_color.rgb * ( ambient + volume * inclination )` once, after the loop, where `inclination` is `abs( dot( direction, best_hit.hit_normal ) )`.

## Parameters

- `figure_item.parameters` reaches the block as the fixed slot array of the shared payload.
- The block reads the slots declared by its figure trait `m_parameter_count`; the host asserts that the count fits the shared `parameter_capacity` and fills the unused slots with `0.0f`.

## Prelude Helpers

- The fragment template provides these helpers to every block:

| Helper | Purpose |
|---|---|
| `vec2_from( data, index )` | Reads two consecutive floats as a `vec2` |
| `vec3_from( data, index )` | Reads three consecutive floats as a `vec3` |
| `vec4_from( data, index )` | Reads four consecutive floats as a `vec4` |
| `square( value )` | Squares one value |
| `rotated_1( value )` | Cyclic shift `yzx` |
| `rotated_2( value )` | Cyclic shift `zxy` |
| `min_axis( value )` | Smallest component |
| `max_axis( value )` | Largest component |
| `sum_axis( value )` | Dot product with `vec3( 1.0 )` |
| `no_hit( )` | The shared miss outcome |
| `hit_result` | The hit outcome structure |

## Generated Shape

- The host emits one case per loaded figure, inlines the block text verbatim after the case label and closes the case with a generated `break`:

```glsl
	switch( uint( figure_item.type_id ) )
	{
		case <type_id>u: <the block text of the figure file>
		break;
	}
```

- The block keeps the indentation written in its file.
- The examples in this folder: [sphere.glsl](sphere.glsl) and [triangle.glsl](triangle.glsl).
