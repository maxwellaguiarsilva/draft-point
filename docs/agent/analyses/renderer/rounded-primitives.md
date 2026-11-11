# Rounded Primitives

## Scope

- Analytical edge and corner rounding on a single box primitive controlled by a proportional `border_radius` scalar.
- Half-space cut-plane clipping and interior surface inversion on the same primitive, exposing inner flat walls and concave fillets inside an open cavity.
- Continuous surface curvature and curved outer silhouettes evaluated analytically from ray-surface intersection on a single primitive at fixed geometry count.

## Architectural Layers

| Layer | Mechanism | Geometric Guarantee |
|---|---|---|
| **Convex Hybrid Rounded Box** | Closed rounded box parameterized by outer `half_size` and proportional `border_radius` in `[ 0.0, 1.0 ]` | Continuous inclination shading across core faces and curved fillets with genuinely curved outer silhouettes |
| **Half-Space Cut & Interior Concave Inversion** | Local half-space cut plane `( cut_normal, cut_offset )` clipping the rounded box open and inverting far-slab evaluation | Continuous inward inclination across the exposed single-surface interior cavity |

## Premeditated Geometry & Dimension-Symmetric Formulation

### Proportional Scale Convention

- Geometric parameters in `figure_item.parameters` (`m_parameters` on CPU):
  - Outer bounding half-extent `half_size`.
  - Proportional rounding factor `border_radius` within `[ 0.0, 1.0 ]`.
- Derived world-space extents:
  - Fillet arc radius: `fillet_radius = half_size * border_radius`.
  - Flat inner core half-extent: `core = half_size * ( 1.0 - border_radius )`.
- Continuous shape spectrum under one formula:
  - `border_radius = 0.0` yields `fillet_radius = 0.0` and `core = half_size`, reproducing a sharp cube.
  - `border_radius = 0.1` yields a `10%` inset fillet around a `90%` flat core.
  - `border_radius = 1.0` yields `fillet_radius = half_size` and `core = 0.0`, reproducing an analytical sphere.

### Unified Minkowski Subspace Projection

- Minkowski decomposition:
  - A rounded box is the Minkowski sum of the flat core box `[ -core, core ]^3` and a sphere of radius `fillet_radius`.
- Simultaneous multi-axis Voronoi feature classification at the outer bounding-box entry `entry_point = local_origin + unit_direction * enter_distance`:
  - Signed Voronoi region vector across all spatial dimensions simultaneously: `region = sign( entry_point ) * step( vec3( core ), abs( entry_point ) )`.
  - Active curved-axis selector: `mask = abs( region )`.
  - Inset feature center: `anchor = region * core`, placing the curved feature center across every Voronoi subspace in one vector product.
- Feature classification by active-axis sum `sum_axis( mask )` from the GLSL prelude of [Agnostic Pipeline](agnostic-pipeline.md):
  - `sum_axis( mask ) == 1.0` selects a flat core face where outer slab distance `enter_distance` and flat inclination `dot( abs( unit_direction ), mask )` hold directly.
  - `sum_axis( mask ) == 2.0` selects a cylindrical edge fillet by projecting out the unconstrained edge axis.
  - `sum_axis( mask ) == 3.0` selects a spherical corner octant anchored at `anchor`.
- Subspace quadratic evaluation via the unnormalized-direction sphere discriminant of [Baseline Reference](baseline.md):
  - Projecting the ray into the masked subspace gives `subspace_origin = ( local_origin - anchor ) * mask` and `subspace_direction = unit_direction * mask`.
  - Squared subspace direction norm `direction_norm_squared = dot( subspace_direction, subspace_direction )` equals `1.0` in a 3D corner octant and lies below `1.0` in a 2D cylindrical edge subspace where one axis is masked to `0.0`.
  - Preserving division by `direction_norm_squared` keeps the quadratic discriminant identical across both 3D corners and 2D edge cylinders:
    - `hypotenuse = -subspace_origin`
    - `along = dot( hypotenuse, subspace_direction )`
    - `opposite_leg_squared = dot( hypotenuse, hypotenuse ) - square( along ) / direction_norm_squared`
    - `ratio = opposite_leg_squared / square( fillet_radius )`
  - Silhouette miss and convex near-root hit when `sum_axis( mask ) >= 2.0`:
    - `ratio > 1.0` means the ray passes through the cut-away sharp corner margin outside the fillet, producing a curved outer silhouette miss unless the ray crosses into an adjacent Voronoi region before `exit_distance`.
    - `ratio <= 1.0` yields the near-root convex entry distance `fillet_distance = ( along - sqrt( max( 0.0, direction_norm_squared * ( square( fillet_radius ) - opposite_leg_squared ) ) ) ) / direction_norm_squared`, reported as `hit_distance` once cell membership holds.
    - Outward unit fillet normal `outer_normal = ( ( local_origin + unit_direction * fillet_distance - anchor ) * mask ) / fillet_radius` produces inclination cosine `cos_incidence = abs( dot( unit_direction, outer_normal ) )`, matching adjacent flat core faces where `outer_normal` aligns with `region`.

```glsl
//	unified convex fillet projection across edges and corners
vec3	entry_point = local_origin + unit_direction * enter_distance;
vec3	region = sign( entry_point ) * step( vec3( core ), abs( entry_point ) );
vec3	mask = abs( region );
vec3	anchor = region * core;
vec3	subspace_origin = ( local_origin - anchor ) * mask;
vec3	subspace_direction = unit_direction * mask;
float	direction_norm_squared = dot( subspace_direction, subspace_direction );
vec3	hypotenuse = -subspace_origin;
float	along = dot( hypotenuse, subspace_direction );
float	opposite_leg_squared = dot( hypotenuse, hypotenuse ) - square( along ) / direction_norm_squared;
float	ratio = opposite_leg_squared / square( fillet_radius );
float	root_span = sqrt( max( 0.0, direction_norm_squared * ( square( fillet_radius ) - opposite_leg_squared ) ) );
float	fillet_distance = ( along - root_span ) / direction_norm_squared;
vec3	outer_normal = ( ( local_origin + unit_direction * fillet_distance - anchor ) * mask ) / fillet_radius;
float	cos_incidence = abs( dot( unit_direction, outer_normal ) );
```

- Convex grazing Voronoi transition guard:
  - A grazing ray entering the outer bounding box inside a corner octant (`sum_axis( mask ) == 3.0`) or edge cell (`sum_axis( mask ) == 2.0`) can miss the initial curved feature or cross a core plane `abs( coordinate ) == core` into an adjacent edge cylinder or flat core face prior to `exit_distance`.
  - Checking `step( vec3( core ), abs( fillet_point ) ) == mask` at `fillet_point = local_origin + unit_direction * fillet_distance` validates whether the intersection stays inside the active Voronoi cell.
  - Stepping across core-plane slab crossings updates `region` and `mask` across at most two convex Voronoi transitions along any straight ray, or delegates to a closed-form analytical rounded-box root solver.

### Interior Concave Fillet Inversion

- Open-cavity cut-plane semantics:
  - Local half-space plane `( cut_normal, cut_offset )` clips away points satisfying `dot( local_point, cut_normal ) > cut_offset` and retains points satisfying `dot( local_point, cut_normal ) <= cut_offset`.
  - Single-surface hollow cavity representation exposes the interior back walls and concave fillets directly through the clipped opening.
- Mathematical inversion from convex entry (`enter_distance`, near root) to concave exit (`exit_distance`, far root):
  - Retained outer hit: when the outer convex hit `convex_distance > 0.0` at `convex_point = local_origin + unit_direction * convex_distance` satisfies `dot( convex_point, cut_normal ) <= cut_offset`, the convex outer surface reports `hit_distance = convex_distance` directly.
  - Interior cavity traversal: when the front hit is clipped away by the cut plane or the ray originates inside the bounding box (`enter_distance < 0.0`), evaluation advances to the far bounding-box slab exit `exit_distance`.
  - Initial Voronoi classification at `exit_point = local_origin + unit_direction * exit_distance` evaluates the masked subspace quantities:
    - `region = sign( exit_point ) * step( vec3( core ), abs( exit_point ) )`
    - `mask = abs( region )`
    - `anchor = region * core`
  - Far quadratic root (`+ root_span`) on curved inner features (`sum_axis( mask ) >= 2.0` and `ratio <= 1.0`):
    - `concave_distance = ( along + root_span ) / direction_norm_squared`
    - Because the inner fillet curves inward toward the figure center relative to the outer bounding box, every ray traversing the interior cavity intersects an interior feature at `concave_distance <= exit_distance`.
  - Backward Voronoi cell transition guard:
    - When `exit_point = local_origin + unit_direction * exit_distance` falls in a corner (`sum_axis( mask ) == 3.0`) or edge (`sum_axis( mask ) == 2.0`) cell, moving backward along the ray to the far root `concave_distance <= exit_distance` can land at `concave_point = local_origin + unit_direction * concave_distance` with `abs( concave_point ) < core` on one or more axes (`step( vec3( core ), abs( concave_point ) ) != mask`).
    - Re-evaluating `region = sign( concave_point ) * step( vec3( core ), abs( concave_point ) )` and `mask = abs( region )` at `concave_point` and recomputing the masked subspace distance resolves every interior cavity ray to exact zero SDF residual across at most two monotone transitions:
      - Corner octant (`sum_axis( mask ) == 3.0`) to concave edge cylinder (`sum_axis( mask ) == 2.0`).
      - Concave edge cylinder (`sum_axis( mask ) == 2.0`) to inner flat wall (`sum_axis( mask ) == 1.0`).
  - Visibility confirmation and inward normal continuity:
    - Evaluating `concave_point = local_origin + unit_direction * concave_distance` confirms a visible interior hit whenever `concave_distance > 0.0` and `dot( concave_point, cut_normal ) <= cut_offset`.
    - Outer scene depth sorting receives `hit_distance = concave_distance` directly along the ray.
    - Inward unit normal `inner_normal = -outer_normal` yields `cos_incidence = abs( dot( unit_direction, inner_normal ) )`, maintaining continuous inclination across all interior Voronoi regions:
      - Inner flat core walls (`sum_axis( mask ) == 1.0`).
      - Concave cylindrical edge fillets (`sum_axis( mask ) == 2.0`).
      - Concave spherical corner octants (`sum_axis( mask ) == 3.0`).

## Open Questions

| Topic | Option A | Option B | Trade-Off |
|---|---|---|---|
| **Figure registration** | Single parameterized rounded-box figure where a sentinel `cut_offset` keeps the primitive closed | Separate convex rounded-box and cut-cavity rounded-box figure types in the dispatch table | Option A shares one shader body across closed and cut instances; Option B isolates cut-plane parameters in `m_parameters` to cavities |
| **Convex grazing Voronoi transition** | Bounded two-step slab advance across core planes `abs( coordinate ) == core` | Closed-form analytical rounded-box root solver with explicit clamped-core normal check | Option A reuses the masked subspace quadratic exclusively; Option B resolves convex grazing transitions in a single pass |

## Boundary

- Screen ray projection and inclination brightness formula belong to [Baseline Reference](baseline.md).
- Flat SSBO payload stride rules and per-figure shader injection belong to [Agnostic Pipeline](agnostic-pipeline.md).
- Interior cavity clipping models a single-surface open cavity; solid wall thickness and boolean cap faces belong to solid-modeling extensions outside this domain.
- [Adaptive Mesh](adaptive-mesh.md) owns:
  - Arbitrary polygon mesh topology.
  - Heterogeneous feature-edge lengths.
  - Per-vertex proportional radius clamping.
- Rigid 3D rotation of the rounded box and its local cut plane belongs to [Orientation](orientation.md).

## Relations

- Parent domain: [3D Renderer](index.md)
- Upstream inclination brightness contract: [Baseline Reference](baseline.md)
- Upstream figure dispatch and GLSL prelude contract: [Agnostic Pipeline](agnostic-pipeline.md)
- Downstream dihedral mesh crease generalization: [Adaptive Mesh](adaptive-mesh.md)
- Orthogonal local-frame ray rotation: [Orientation](orientation.md)
