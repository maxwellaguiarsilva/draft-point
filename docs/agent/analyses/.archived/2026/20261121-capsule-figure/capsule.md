# Capsule Figure

## Scope

- Archived on 2026-11-21 by explicit user request.
- The capsule leaves the Stage 2 experiment while its knowledge is kept in this document.
- The evaluator is archived verbatim at `capsule.glsl` beside this document.
- The C++ trait, class, registration, and `figure_traits` tuple entry are removed from the prototype.
- The active experiment continues with the curved `sphere` and the planar 3D `triangle` only.

## CPU Contract at Archive Time

| Element | Contract |
|---|---|
| Trait | `figure_trait::capsule` with `m_parameter_count = 5uz` |
| Class | `capsule : vertex_shape< 2 >` |
| Storage | `capsule::data = array< position, 2 >` holding absolute world endpoints |
| Radius | `m_radius` |
| `position( )` | Midpoint of the two endpoints |
| `position( value )` | Translates both endpoints by `value` |
| `parameters( )` | `unit_axis` (3 floats), `half_length` (`0.5f * length( segment )`), `radius` |

## GPU Parameter Layout

| Payload Field | Slot | Content |
|---|---|---|
| `m_parameters` | `parameters[ 0..2 ]` | Unit axis of the segment |
| `m_parameters` | `parameters[ 3 ]` | `half_length` |
| `m_parameters` | `parameters[ 4 ]` | `radius` |
| `m_position` | `position` | Midpoint of the two endpoints |

## Intersection Math

- Three branches evaluated in one loop over `branch_index`:
  - `branch_index = 0`: finite cylinder body.
  - `branch_index = 1`: hemispherical endcap with `cap_sign = -1.0`.
  - `branch_index = 2`: hemispherical endcap with `cap_sign = 1.0`.
- Per-branch closest approach:
  - `branch_local` is the ray origin to the branch center offset: the perpendicular center component for the cylinder body, and `local_center + ( cap_sign * half_length ) * axis` for the endcaps.
  - `branch_along` projects `branch_local` onto the branch direction: the perpendicular direction for the cylinder body, and `unit_direction` for the endcaps.
  - `opposite_leg_squared` is the squared perpendicular offset of the branch center from the branch ray.
  - `cross_cosine` is `sqrt( 1.0 - clamp( opposite_leg_squared / radius_squared, 0.0, 1.0 ) )`.
- Chord half-length:
  - `half_chord = radius * cross_cosine / direction_scale`, where `direction_scale` is the perpendicular direction length for the cylinder body and `1.0` for the endcaps.
  - `center_distance = branch_along / direction_scale`.
- Acceptance per branch:
  - The cylinder body branch is skipped when `perpendicular_direction_squared <= 1e-12`.
  - Both candidate roots (`center_distance - half_chord` and `center_distance + half_chord`) are tested.
  - Cylinder axial containment: `abs( candidate * direction_along - center_along ) <= half_length`.
  - Cap hemisphere sign test: `cap_sign * dot( candidate * unit_direction - branch_local, axis ) >= 0.0`.
- Nearest accepted hit:
  - The first accepted candidate below `best_distance` replaces it, and the traversal ends with `found ? hit_result( ... ) : no_hit( )`.

## Hit Normal Under the Current Contract

- Knowledge that was never implemented: the capsule archival precedes the `hit_normal` contract, so this section records how the shape maps onto it.
- Both normals are already unit and require no `normalize` call.
- Shared input: `hit_local = hit_distance * unit_direction - local_center`.
- Cylinder body branch:
  - Radial component only: `( hit_local - dot( hit_local, axis ) * axis ) / radius`.
- Endcap branch:
  - Sphere centered at the cap center: `( hit_local - cap_sign * half_length * axis ) / radius`.
- Shading factor ownership:
  - The dispatcher derives `inclination = abs( dot( unit_direction, hit_normal ) )` and shades `color.rgb * ( ambient + volume * inclination )`.
  - The evaluator returns the normal only, never the lighting factor.

## Archive-Time Contract Note

- The archived `capsule.glsl` still returns the older signature `hit_result( bool hit, float hit_distance, float cos_incidence )`, accumulating the third field in `best_cosine`.
- The current `hit_result` carries `bool hit`, `float hit_distance`, and `vec3 hit_normal`.
- Reinstating the figure requires the third return field to become the `hit_normal` described above, with the lighting factor left to the dispatcher.

## Reinstatement Checklist

1. Move `capsule.glsl` back to `data/shader/figures/`.
2. Restore the `figure_trait::capsule` trait and the `capsule` class in the prototype.
3. Add the trait to `figure_traits`.
4. Adapt the evaluator return to the current `hit_result`.
5. Re-check the `m_type_id` values, which are assigned from the sorted file stems.

## Relations

- Archived from [Agnostic Pipeline](../../../renderer/agnostic-pipeline.md).
- Archived evaluator: `capsule.glsl` in this directory.
- Parent analysis tree: [Analyses](../../../index.md).
