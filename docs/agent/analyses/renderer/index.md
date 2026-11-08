# 3D Renderer

## Scope

- Long-term architecture and staged roadmap for a renderer that rounds sharp edges and sharp corners of legacy 3D games automatically.
- The rounding is analytical: surface curvature is derived from the ray-surface inclination, not from triangle tessellation, so a polygon keeps its low polygon count while its border renders continuous.
- Shading pairs the analytical inclination with one flat dominant color per polygon, replacing photographic textures.
- The domain stops at the renderer and its input data. It does not own game logic, asset extraction, or the ethical scope of the source game, which are recorded in [Legacy Game Preservation](pickup-express.md).

## Invariants

- Every stage is delivered only when implemented under the `draft-point` rules. The primary rule is the [dimension symmetry invariant](#dimension-symmetry-invariant): a delivery that reaches the visual goal while unrolling multidimensional operations is not a delivery.
- Prototypes are isolated executables created with `cpp_create_test` (`flg_adhoc = true`) and compiled with `cpp_compile`. No stage is developed directly inside an application binary.

### Dimension Symmetry Invariant

- Multidimensional formulas stay written once and are applied per dimension by the `sak` pipelines (`rotated`, `zip`, `zip_transform`, `views`, niebloids, `to`).
- Opening coordinates by hand (`[ 0 ]`, `[ 1 ]`, `.x`, `.y`, `.z`) inside a formula that later gets repeated per axis defeats the purpose of the project.
- Evidence of the intended form: `include/sak/math/vector.hpp`, `__cross::operator( )` is written as `rotated( left, 1 ) * rotated( right, 2 ) - rotated( left, 2 ) * rotated( right, 1 )` instead of three hand-written component products.

## Stage Roadmap

| Order | Stage | Document |
|---|---|---|
| 0 | Reference contract of the current sphere renderer | [Baseline Reference](baseline.md) |
| 1 | Move `sak::point` member operations into niebloids | [Point Niebloid Purification](point-niebloids.md) |
| 2 | Geometry-agnostic CPU/GLSL pipeline with injected intersection formulas | [Agnostic Pipeline](agnostic-pipeline.md) |
| 3 | Hybrid rounded cube and interior (concave) inversion | [Rounded Primitives](rounded-primitives.md) |
| 4 | Adaptive edge-rounding triangle mesh | [Adaptive Mesh](adaptive-mesh.md) |
| 5 | Quaternion orientation reduced to a CPU-built `mat3` on the GPU | [Orientation](orientation.md) |
| 6 | Doubly-linked spatial graph and incremental visible-set discovery | [Spatial Graph](spatial-graph.md) |
| 7 | Legacy game meshes with flat dominant-color shading | [Legacy Game Preservation](pickup-express.md) |

- The order is meaningful: each stage consumes the primitive contract produced by the previous one. Stage 3 needs the injected formula dispatch of stage 2; stage 4 needs the hybrid primitive of stage 3; stages 5 and 6 are independent of each other but both need stage 4 meshes to be worth validating.

## Relations

- Resume work from [Agent Status](../../status.md), which holds the single next task and the commands that reproduce the current state.
- The domain was created in the session that produced these documents. Its parent node is registered in [Analyses](../index.md).

## Document Roles

- Facts about the current code live in the stage documents, each citing the files that support them.
- Decisions taken by the owner live in the stage document that the decision constrains.
- Undecided questions are collected under "Open Questions" inside the affected stage document and are never presented as decided design.
- Session state, next task and reproduction commands live only in [Agent Status](../../status.md).
