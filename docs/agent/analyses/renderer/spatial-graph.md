# Spatial Graph

## Scope

- CPU doubly-linked spatial proximity graph that tracks moving objects and a non-rendered camera node incrementally, feeding visible object indices to the GPU per frame.
- Defines the exploratory CPU spatial-graph model and enumerates the geometric invariants required for incremental visible-set discovery.
- Decouples CPU visibility discovery from GPU surface evaluation so the fragment shader loops over candidate visible figures while keeping world figure payloads persistent.

## Architectural Layers

| Layer | Mechanism | Verification Target |
|---|---|---|
| **Pure Spatial Proximity Topology & Incremental Local Rewiring** | Symmetric doubly-linked CPU proximity graph maintaining global connectivity through local neighbor walks upon node translation | CPU 2D-to-3D unit and TUI ad-hoc verification of bidirectional link symmetry and local-walk convergence |
| **Camera Boundary Tracking & GPU Visible-Index Indirection** | Non-rendered camera node updating frustum-boundary nodes incrementally and streaming active figure identifiers to a `visible_indices` SSBO | Bounded region collection under camera translation and rotation paired with indirect `figure_ssbo` lookup on GPU |

## Core Architectural Mechanics

- **Doubly-Linked Proximity Topology**:
  - Every world object is a graph node holding bidirectional links to spatially adjacent nodes.
  - Moving an object traverses its own neighbor links to locate its updated neighborhood and rewires locally in place.
- **Camera as Non-Rendered Graph Node**:
  - The camera participates as a node in the same proximity graph and updates its neighborhood incrementally as it moves.
  - Camera exclusion from surface rendering holds either via a node visibility attribute (`is_renderable = false`) or by storing the camera node outside the renderable `figure_ssbo` slice.
- **Boundary-Pointer Tracking**:
  - The camera tracks extreme visible nodes along the view frustum boundary, modeled from lateral frustum extremes such as the upper-left farthest visible object and the lower-left farthest visible object.
  - Camera translation and rotation walk incrementally from the camera node and the previous boundary nodes to locate the updated frustum extremes.
- **Per-Frame Region Traversal**:
  - Prior to rendering each frame, a CPU graph traversal bounded by the camera node and the tracked boundary nodes collects the candidate object indices for GPU evaluation.

## Premeditated Design Mechanics & Geometric Invariants

- **Dimension-Symmetric Neighbor Links**:
  - Each node stores an axis-independent, symmetric bidirectional neighbor collection supporting navigation across arbitrary movement directions and diagonals.
  - Directional navigation advances to the neighbor maximizing `dot( normalize( neighbor_position - current_position ), search_direction )` or monotonically decreasing squared target distance `dot( neighbor_position - target_position )`.
- **Connectivity & Local-Minima Guarantee**:
  - Greedy spatial walks require a single connected proximity graph free of void-induced dead ends so every target outside a node's immediate neighborhood admits a progress-making neighbor step.
  - Candidate proximity graph families providing navigable local routing:
    - Gabriel graphs.
    - Relative Neighborhood Graphs.
    - Delaunay-style local edge-flip graphs.
    - Angularly distributed symmetric sector links.
- **Frustum Coverage Geometry**:
  - Two left-side corners (`upper_left`, `lower_left`) bound a 1D vertical span along one lateral side of the view cone.
  - Covering a full 3D perspective frustum requires symmetric corner extremes across $2^{\text{dimensions} - 1}$ frustum rays (four frustum corners in 3D, or two opposite lateral corners in a 2D ground-plane projection when vertical elevation variance is bounded).
  - Frustum plane tests include a conservative angular or metric margin scaled by each object's `bounding_radius` so objects whose volumes intersect the view cone remain selected when their centers sit outside a frustum plane.
- **Empty-Space & Sky Horizon Handling**:
  - Frustum rays aimed into open sky or past the world perimeter resolve through a perimeter-hull horizon fallback.
  - Boundary walks clamp to the farthest reachable graph perimeter node along the ray direction within `max_view_distance`, and emit an empty visible set when all graph hull nodes lie behind the camera frustum.
- **Zero-Copy GPU Indirection (`visible_indices` SSBO)**:
  - `figure_ssbo` (`binding = 2`) stays persistent in world index order on the GPU, updating exclusively the payload slices of objects that moved or reoriented during the frame.
  - Each frame uploads a compact `visible_indices` SSBO (`binding = 4`) carrying a header count followed by exact integer-valued `float` object identifiers.
  - The fragment shader outer loop resolves each visible candidate through indirect lookup:
    - Iterates `slot` across `[ 0u, visible_ssbo.count )`.
    - Decodes `uint figure_index = uint( visible_ssbo.data[ slot ] );`.
    - Unpacks `figure_item = ssbo_figure( figure_index );` including `figure_item.parameters`.
    - Evaluates surface intersection along `unit_direction` for `hit_distance` depth sorting.

## Open Questions

- Selection among the candidate proximity graph families to balance incremental local rewiring cost against guaranteed greedy-walk convergence in 3D.
- Choice between full four-corner 3D frustum boundary tracking and two-corner 2D ground-plane boundary tracking for scenes with bounded vertical elevation.
- Choice of interior region collection algorithm between visited-epoch breadth-first flood bounded by frustum planes and boundary-to-camera hull sweep.
- Bootstrap insertion sequence for constructing the initial spatial graph incrementally via local walks from an initial seed node at scene load.

## Boundary

- Base `figure_ssbo` payload contract (`figure_payload` with `m_parameters`) and shader injection belong to [Agnostic Pipeline](agnostic-pipeline.md).
- Analytical primitive rounding belongs to [Rounded Primitives](rounded-primitives.md).
- Adaptive mesh topology and secondary `triangle_ssbo` payloads belong to [Adaptive Mesh](adaptive-mesh.md).
- Quaternion composition and per-figure `orientation_basis` slots belong to [Orientation](orientation.md).
- This specification owns:
  - CPU spatial graph topology and incremental local rewiring.
  - Camera node integration and incremental frustum-boundary tracking.
  - Visible-region index collection and the `visible_indices` SSBO indirection contract.
- Multi-object legacy scene population belongs to [Legacy Game Preservation](pickup-express.md).
- Extensions outside this specification:
  - Geometric level-of-detail switching.
  - Opaque-wall occlusion culling.
  - Multi-frame traversal caching.

## Relations

- Parent domain: [3D Renderer](index.md)
- Upstream Euclidean distance and vector predicates: [Point Niebloid Purification](point-niebloids.md)
- Upstream persistent `figure_ssbo` indexing contract: [Agnostic Pipeline](agnostic-pipeline.md)
- Bounded mesh evaluation payloads: [Adaptive Mesh](adaptive-mesh.md)
- Orthogonal persistent world-slot orientation: [Orientation](orientation.md)
- Downstream multi-object town scene consumer: [Legacy Game Preservation](pickup-express.md)
