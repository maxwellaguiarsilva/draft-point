# 3D Renderer

## Scope

- Architectural domain for an analytical 3D renderer that rounds sharp edges and sharp corners of legacy 3D games:
  - Surface curvature derives analytically from camera-ray inclination against continuous mathematical primitive surfaces.
  - Low-polygon meshes retain their original polygon counts while edges and corners render with continuous rounded profiles.
  - Surface shading multiplies analytical ray-surface inclination by one flat dominant color per polygon.
- Domain boundary:
  - Core rendering architecture owns:
    - Flat-float GPU storage contracts.
    - CPU fragment shader assembly and dispatch injection.
    - Analytical ray-surface intersection and inclination math.
    - CPU quaternion orientation reduced to a local-frame basis on the GPU.
    - Doubly-linked spatial proximity visibility discovery.
  - Legacy game preservation ethics and `.ccf` asset preparation belong to [Legacy Game Preservation](pickup-express.md).

## Core Invariants

| Invariant | Governing Contract | Canonical Evidence |
|---|---|---|
| Dimension Symmetry (C++) | Multidimensional operations execute once over whole vectors and coordinate ranges through `sak` range niebloids and views | `include/sak/math/vector.hpp` (`__cross`), `include/sak/geometry/point.hpp` |
| Dimension Symmetry (GLSL) | Primitive formulas operate on whole-vector GLSL intrinsics and a single fragment shader prelude | [Agnostic Pipeline](agnostic-pipeline.md) |
| Ad-hoc Prototype Isolation | Each stage lives in its own standalone prototype translation unit under `tests/adhoc/rendering/`, keeping baseline sources unchanged | `source/sdl3_shadertoy.cpp`, `data/shader/sdl3-shadertoy.glsl` |
| Flat-Float SSBO Protocol | GPU buffers use `sak::opengl::shader::storage_buffer< t_payload >` constrained by `is_flat`, prefixed with `{ uint count; uint stride; }` in float units over `float data[ ];` | `include/sak/opengl/shader.hpp`, [SSBO Reflection](../../ssbo-reflection.md) |
| Analytical Surface-Ray Inclination | Fragment brightness equals `ambient + volume * cos_incidence` where `cos_incidence = abs( dot( unit_direction, surface_normal ) )` directly from camera-ray geometry | [Baseline Reference](baseline.md), [Agnostic Pipeline](agnostic-pipeline.md) |

### Dimension Symmetry (C++ and GLSL)

- C++ host formulas:
  - Multidimensional operations compose over whole coordinate ranges through `sak` niebloids and range adaptors:
    - `rotated`
    - `zip`
    - `zip_transform`
    - `views`
    - `to`
  - Reference formulation in `include/sak/math/vector.hpp` expresses `__cross::operator ( )` as `rotated( left, 1 ) * rotated( right, 2 ) - rotated( left, 2 ) * rotated( right, 1 )`.
- GLSL device formulas:
  - Vector expressions apply component-wise across multi-component GLSL vector operands through built-in vector intrinsics.
  - Shared fragment shader prelude isolates horizontal axis reductions and cyclic permutations in one definition:
    - `rotated_1`
    - `rotated_2`
    - `min_axis`
    - `max_axis`
    - `sum_axis`
  - Primitive evaluators express intersection and inclination shading strictly through whole-vector intrinsics and those prelude helpers.

### Ad-hoc Prototype Isolation

- Each stage lives in its own standalone prototype translation unit under `tests/adhoc/rendering/`, keeping `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` unchanged as the baseline contract.
- SDL3/OpenGL prototype translation units declare `//	using	config/sdl3.json` and `#define GLAD_GL_IMPLEMENTATION` at the top of the file.

### Flat-Float SSBO Protocol

- CPU-to-GPU transfers go through `sak::opengl::shader::storage_buffer< t_payload >` (`include/sak/opengl/shader.hpp`).
- Every `t_payload` satisfies `is_flat< t_payload >`, combining `has_no_padding( ^^t_payload )` and `is_trivially_copyable_v< t_payload >`.
- Each buffer starts with `ssbo_header{ GLuint m_count; GLuint m_stride; }` where `m_stride` is measured in `float` units, unpacked on the GPU from `float data[ ];` ([SSBO Reflection](../../ssbo-reflection.md)).

### Analytical Surface-Ray Inclination

- Every primitive computes `cos_incidence` in `[ 0.0, 1.0 ]` and shades fragments as `color.rgb * ( ambient + volume * cos_incidence )`.
- On curved primitives, `cos_incidence = sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) )` evaluates `abs( dot( unit_direction, surface_normal ) )` directly from radial closest-approach geometry at the ray entry point.
- On flat faces, perspective ray divergence across a face and between adjacent faces yields `cos_incidence = abs( dot( unit_direction, face_normal ) )`, separating front and side planes directly from camera-ray inclination.

## Nodes

- [Baseline Reference](baseline.md) — Reference sphere renderer contract and flat-float storage buffer layout.
- [Point Niebloid Purification](point-niebloids.md) — Coordinate-storage `sak::point` class and external dimension-symmetric niebloids.
- [Agnostic Pipeline](agnostic-pipeline.md) [#current] — Heterogeneous flat figure payload and CPU-injected GLSL intersection dispatch.
- [Rounded Primitives](rounded-primitives.md) — Convex Minkowski rounded cube and interior concave fillet inversion.
- [Adaptive Mesh](adaptive-mesh.md) — Per-corner clamped border radius and dihedral crease rounding on triangle meshes.
- [Orientation](orientation.md) — Dimension-symmetric CPU quaternion algebra reduced to a local-frame basis on the GPU.
- [Spatial Graph](spatial-graph.md) — Doubly-linked spatial proximity graph and incremental visible-index discovery.
- [Legacy Game Preservation](pickup-express.md) — Ethical preservation scope and flat dominant-color validation on legacy meshes.

## Relations

- Parent analysis tree: [Analyses](../index.md)
- Session state and task execution: [Agent Status](../../status.md)
- Flat-float GPU buffer protocol and C++26 reflection architecture: [SSBO Reflection](../../ssbo-reflection.md)
