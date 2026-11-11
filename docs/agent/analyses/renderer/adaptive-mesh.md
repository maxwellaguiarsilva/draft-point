# Adaptive Mesh

## Scope

- Generalizes the analytical edge and corner rounding of [Rounded Primitives](rounded-primitives.md) to arbitrary triangle meshes with heterogeneous edge lengths.
- Preserves exact input triangle counts while analytical ray-surface evaluation produces continuous curvature across mesh creases and vertices.
- Validation centers on a 3D arrow model exercising six geometric regimes:
  - Long shaft edges.
  - Medium arrowhead barb edges.
  - Short tip edges.
  - Coplanar triangulated quad diagonals.
  - Convex outer ridges.
  - Concave shaft-to-barb notches.

## Architectural Layers

| Layer | Mechanism | Verification Target |
|---|---|---|
| **Flat Analytical Triangle Mesh** | Secondary flat `triangle_ssbo` (`binding = 3`) paired with a `mesh` figure descriptor in `figure_ssbo` (`m_parameters`) and dimension-symmetric ray-triangle intersection | Sharp 3D arrow with exact `hit_distance` sorting and flat `abs( dot( unit_direction, face_normal ) )` inclination shading |
| **CPU Manifold Topology & Clamped Radius Baker** | Deterministic `sak` pipeline extracting manifold feature edges and baking per-vertex clamped radii | CPU unit verification of topological crease classification and proportional minimum-edge clamping |
| **GPU Dihedral Crease & Corner Fillet Evaluation** | Fragment evaluation of interpolated edge cylinders and vertex fillets across convex ridges and concave notches | Continuous inclination shading and curved silhouettes across every edge-length regime of the 3D arrow |

## Premeditated Rules & Mathematical Mechanics

### Global Proportion & Per-Corner Clamp

- Global proportion source:
  - `environment` stores one global proportional `m_border_radius` scalar (for example `0.1f` representing `10%`).
- Per-vertex clamping formula:
  - At each mesh `vertex`, `local_radius( vertex ) = global_border_radius * min( length( edge ) )` evaluated over every active feature `edge` incident to `vertex`.
- Worked clamp invariant:
  - Incident active feature-edge lengths `{ 10.0f, 7.0f, 5.0f }` meeting at `vertex` under `global_border_radius = 0.1f` yield `local_radius( vertex ) = 5.0f * 0.1f = 0.5f`.
  - Vertices surrounded by long edges round broadly, while vertices constrained by a short incident edge clamp locally to fit that corner.

### Per-Edge Linear Variation & Barycentric Extension

- Linear edge variation:
  - Active feature `edge` connecting `first` and `second` with clamped radii `radius_first` and `radius_second` interpolates along normalized `edge_factor` in `[ 0.0f, 1.0f ]` via `edge_radius( edge_factor ) = mix( radius_first, radius_second, edge_factor )`.
- Barycentric face extension:
  - Across triangle `( first, second, third )` with barycentric weights `( weight_first, weight_second, weight_third )`, vertex radii extend continuously via `weight_first * radius_first + weight_second * radius_second + weight_third * radius_third`.
  - Along any triangle edge where the opposite barycentric weight equals `0.0f`, the barycentric sum reduces identically to `mix( radius_first, radius_second, edge_factor )`, guaranteeing continuous radius interpolation across shared edges.

### Premeditated Topology & Dihedral Mechanics

- **Coplanar Triangulation Diagonals**:
  - Rectangular mesh faces (such as the shaft walls and arrowhead barb quads of a 3D arrow) split into triangle pairs sharing an internal diagonal with parallel outward normals (`dot( normal_first, normal_second ) -> 1.0f`).
  - Filtering edges satisfying `1.0f - dot( normal_first, normal_second ) <= crease_epsilon` out of the active feature-edge set keeps triangulated quads flat and excludes internal diagonal lengths from the per-vertex minimum clamp.
- **Dihedral Crease Sign (Convex Ridge vs Concave Notch)**:
  - Adjacent triangles sharing oriented `unit_edge = normalize( second - first )` with outward unit normals `normal_first`, `normal_second` and centroid offset `centroid_offset = center_second - center_first` carry `crease_sign = sign( dot( normal_first, -centroid_offset ) )` (equivalently `sign( dot( cross( normal_first, normal_second ), unit_edge ) )` under consistent counter-clockwise winding).
  - **Convex crease** (`crease_sign > 0.0f`):
    - Applies to outer ridges such as arrow shaft edges and the arrow tip.
    - Shaves the sharp ridge inward using the convex near-root evaluation of [Rounded Primitives](rounded-primitives.md), so grazing rays in the shaved margin miss the triangle pair and produce a curved outer silhouette.
  - **Concave crease** (`crease_sign < 0.0f`):
    - Applies to interior wedges such as the two inner notches where the arrow shaft meets the wider arrowhead barbs.
    - Fills the interior wedge outward toward the ray using the concave far-root evaluation of [Rounded Primitives](rounded-primitives.md), so rays aimed into an inner notch hit the concave cylindrical fillet ahead of either flat triangle plane.
- **Dihedral Angle Scale & Altitude Clamp**:
  - At an edge whose adjacent faces form interior dihedral angle `2.0f * half_dihedral_angle` (`cos( 2.0f * half_dihedral_angle ) = -dot( normal_first, normal_second )`), tangent `inset_distance` from the edge relates to `fillet_radius` by `fillet_radius = inset_distance * tan( half_dihedral_angle )`.
  - Orthogonal faces yield `tan( half_dihedral_angle ) = 1.0f`, giving `fillet_radius = inset_distance` in exact agreement with [Rounded Primitives](rounded-primitives.md).
  - Clamping `inset_distance` to a bounded fraction of each triangle's altitude keeps opposite edge fillets disjoint on narrow triangles.
- **Open Boundary Edges**:
  - Single-incident boundary edges on single-sided sheet geometry (used by thin-sheet elements in [Legacy Game Preservation](pickup-express.md)) carry an explicit baked `edge_kind` flag selecting either sharp boundary termination or thin-sheet cap rounding.

### Dimension-Symmetric Ray-Triangle Intersection

- Ray-triangle evaluation composes three whole-vector operations across all coordinates simultaneously:
  - Vector subtraction across triangle vertices and `local_origin`.
  - Cyclic `cross` (`rotated( 1 )` and `rotated( 2 )` in `sak::math`, `rotated_1` and `rotated_2` in GLSL).
  - Horizontal `dot` (`sum( first * second )`).

```cpp
//	dimension-symmetric moller-trumbore ray-triangle evaluation
const point< float, 3 > edge_first = second - first;
const point< float, 3 > edge_second = third - first;
const point< float, 3 > direction_cross_second = cross( unit_direction, edge_second ) | to;
const float determinant = dot( edge_first, direction_cross_second );
const float inverse_determinant = 1.0f / determinant;
const point< float, 3 > origin_offset = local_origin - first;
const float weight_second = dot( origin_offset, direction_cross_second ) * inverse_determinant;
const point< float, 3 > offset_cross_first = cross( origin_offset, edge_first ) | to;
const float weight_third = dot( unit_direction, offset_cross_first ) * inverse_determinant;
const float weight_first = 1.0f - weight_second - weight_third;
const float hit_distance = dot( edge_second, offset_cross_first ) * inverse_determinant;
const point< float, 3 > face_normal = cross( edge_first, edge_second ) | normalize | to;
```

### GPU Buffer Layout

- Primary `figure_ssbo` (`mesh` figure descriptor stored in `m_parameters` / `figure_item.parameters`):
  - `first_triangle`: starting index in `triangle_ssbo`.
  - `triangle_count`: contiguous triangle count in `triangle_ssbo`.
  - `bounding_radius`: conservative local-space bounding sphere radius tested prior to iterating `[ first_triangle, first_triangle + triangle_count )`.
- Secondary `triangle_ssbo` (`binding = 3`, satisfying `sak::opengl::is_flat`):
  - Contiguous flat per-triangle records baked once on the CPU containing:
    - Three vertex positions (`first`, `second`, `third`).
    - Flat polygon base color.
    - Outward unit `face_normal`.
    - Per-vertex clamped radii (`radius_first`, `radius_second`, `radius_third`).
    - Per-edge neighbor normals and baked `edge_kind` flags.

## Validation

- Primary visual acceptance target is a 3D arrow mesh rendered through the adaptive rounding pipeline:
  - Broad rounding on long shaft edges.
  - Intermediate rounding on medium arrowhead barb edges.
  - Tightly clamped rounding at short arrow tip edges.
  - Flat uninterrupted surfaces across coplanar triangulated shaft and barb quads.
  - Curved outer silhouettes along convex ridges paired with smooth inward concave fillets inside the two shaft-to-barb notches.
- CPU topology baker unit tests verify:
  - Clamped vertex radius calculation (`{ 10.0f, 7.0f, 5.0f }` with `0.1f` yielding `0.5f`).
  - Linear `edge_radius( edge_factor )` interpolation along active edges.
  - Coplanar quad diagonal exclusion.
  - Convex versus concave dihedral `crease_sign` classification.

## Open Questions

| Topic | Option A | Option B | Trade-Off |
|---|---|---|---|
| **Concave notch candidate envelope** | Expand per-triangle ray-test bounds by the maximum edge fillet inset so rays missing both flat faces near a concave notch still test the wedge cylinder | Emit explicit concave-crease wedge records in a dedicated secondary slice | Option A keeps one uniform `triangle_ssbo` loop; Option B tests concave fillets directly per shared crease |
| **Multi-edge vertex blend (`valence >= 3`)** | Smooth analytical blend of incident edge-cylinder distances weighted at `vertex` | Exact spherical tangent patch anchored at the inset `vertex` center | Option A handles arbitrary vertex valence uniformly; Option B reproduces an exact sphere octant on trivalent corners |
| **Cross-edge color transition across rounded fillets** | Discrete color boundary at the dihedral angle-bisector plane | Smooth fillet-weight color blend matching normal interpolation across the arc | Shared with [Legacy Game Preservation](pickup-express.md): Option A preserves crisp polygon paint regions; Option B softens high-contrast seams between differently colored adjacent faces |

## Boundary

- Convex near-root and concave far-root fillet mathematics originate in [Rounded Primitives](rounded-primitives.md).
- Flat SSBO dispatch and shader source assembly belong to [Agnostic Pipeline](agnostic-pipeline.md).
- Mesh shading pairs analytical inclination with one flat color per triangle; binary asset parsing and CPU dominant texture color reduction belong to [Legacy Game Preservation](pickup-express.md).
- Rigid 3D mesh rotation belongs to [Orientation](orientation.md).
- Scene-level visible object culling belongs to [Spatial Graph](spatial-graph.md).

## Relations

- Parent domain: [3D Renderer](index.md)
- Upstream convex and concave fillet formulas: [Rounded Primitives](rounded-primitives.md)
- Upstream heterogeneous figure dispatch contract: [Agnostic Pipeline](agnostic-pipeline.md)
- Orthogonal local-frame mesh rotation: [Orientation](orientation.md)
- Downstream spatial visibility indexing: [Spatial Graph](spatial-graph.md)
- Downstream preserved-asset mesh consumer: [Legacy Game Preservation](pickup-express.md)
