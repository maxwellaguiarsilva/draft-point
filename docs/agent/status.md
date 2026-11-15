# Agent Status

- Entry-point session document: a session resumes by reading this file first and traversing into linked architecture documents as needed.

## Current Goal

- Build a 3D renderer that rounds sharp edges and sharp corners of legacy 3D games analytically, shaded by surface-ray inclination over flat per-polygon dominant colors, under the `draft-point` dimension-symmetry invariant across C++ and GLSL.
- Domain architecture: [3D Renderer](analyses/renderer/index.md) — active at Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)), where the user shifted the second primitive from sphere + cube to **sphere + trigonometric 3D triangle** (`triangle` class + `triangle_figure` + `evaluate_triangle`), pending implementation and build/runtime verification in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`.

## Next Task

- **Stage 2 — Implement and verify sphere + trigonometric 3D triangle in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`**:
  - Update `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` (preserving the shared template contract in `data/shader/agnostic-pipeline.glsl`) to replace `cube_figure` / slab AABB `evaluate_cube` with:
    - CPU `triangle` class storing absolute vertices in `triangle::data` and precomputing the centroid, three centroid-relative vectors, and the plane unit normal for SSBO delivery.
    - `triangle_figure` trait (`m_parameter_count = 12uz`, `parameter_capacity = 12uz`) and GLSL `evaluate_triangle` performing trigonometric ray-plane intersection, sub-triangle oriented-area membership, and `abs( dot( unit_direction, normal ) )` inclination shading.
  - Run `cpp-include-tree --flg-auto-fix`, build with `cpp_compile`, validate C++ formatting with `cpp_code_verifier`, and run `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` to verify both primitives at runtime.
  - Stages 3 through 7 ([Rounded Primitives](analyses/renderer/rounded-primitives.md), [Adaptive Mesh](analyses/renderer/adaptive-mesh.md), [Orientation](analyses/renderer/orientation.md), [Spatial Graph](analyses/renderer/spatial-graph.md), and [Legacy Game Preservation](analyses/renderer/pickup-express.md)) remain disapproved and held pending revision until Stage 2 is verified and downstream direction is established.

## Reproduction Commands

| Purpose | Command |
|---|---|
| List renderer analysis documents | `find -L docs/agent/analyses/renderer -type f` |
| Inspect working tree state | `git status --porcelain` |
| Valid reference sources (Stage 0 and user triangle-sphere demo) | `source/sdl3_shadertoy.cpp`, `data/shader/sdl3-shadertoy.glsl`, `tests/adhoc/rendering/0010_sdl3_demo.cpp`, and `data/shader/sdl3-demo.glsl` |
| Stage 2 prototype sources (pending trigonometric 3D triangle update and verification) | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` and `data/shader/agnostic-pipeline.glsl` |
| Scaffold isolated stage ad-hoc prototype | `cpp_create_test` (`flg_adhoc = true`, under `tests/adhoc/rendering/`) |
| Mandatory transitive include fix-up before every compile | `cpp-include-tree --flg-auto-fix` |
| Compile project and ad-hoc binaries | `cpp_compile` |
| Verify C++ formatting rules on changed `.hpp` / `.cpp` files | `cpp_code_verifier` |
| Run `sak` geometry and library test suites | `dist/test-sak-geometry` and `dist/test-sak` |
| Publish validated changes | `git_quick_upload` |

- Tools exposed by `project-mcp-tools` (`../project-mcp-tools/README.md`):
  - `cpp_create_test`
  - `cpp_compile`
  - `cpp_code_verifier`
  - `git_quick_upload`
- Running `cpp-include-tree --flg-auto-fix` in the terminal precedes every `cpp_compile` invocation.

## Acceptance Criteria for the Next Task

- **Stage 2 implementation gate**: `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` implements the CPU `triangle` class (`triangle::data` absolute vertices, precomputed centroid, three centroid-relative vectors, plane unit normal), `triangle_figure`, and GLSL `evaluate_triangle` alongside `sphere_figure` and `evaluate_sphere`.
- **Stage 2 build and runtime verification gate**: `cpp-include-tree --flg-auto-fix`, `cpp_compile`, `cpp_code_verifier`, and runtime execution of `0011_agnostic_pipeline` confirm trigonometric ray-plane intersection, sub-triangle oriented-area membership, and `abs( dot( unit_direction, normal ) )` inclination shading.
- **Downstream hold gate**: Stages 3 through 7 remain held pending revision after Stage 2 verification.

## Verified Facts

| Fact | Evidence |
|---|---|
| Stage 0 baseline renderer (`sphere_ssbo` with two spheres using unnormalized `direction` and sphere-center `length( hypotenuse )` depth comparison) remains a valid reference implementation | `source/sdl3_shadertoy.cpp`, `data/shader/sdl3-shadertoy.glsl`, [Baseline Reference](analyses/renderer/baseline.md) |
| User's hand-written sphere triangle example remains a valid reference implementation for the single-triangle ray intersection and inclination-color strategy | `tests/adhoc/rendering/0010_sdl3_demo.cpp`, `data/shader/sdl3-demo.glsl` |
| `sak::point` is pure coordinate storage declaring only `operator ==` and six constructors, including prefix-range + trailing scalars `point( range, first, remaining... )` | `include/sak/geometry/point.hpp` |
| `sak::is_all`, `sak::is_inside_of`, and `sak::is_equal` are free niebloids in flat `namespace sak`; `sak::math::length` and `sak::math::product` are free reduction niebloids in `sak::math` | `include/sak/geometry/point.hpp`, `include/sak/math/vector.hpp`, `include/sak/math/math.hpp` |
| `point` element-wise binary and compound-assignment arithmetic operators are generated externally in `sak::ranges` by `__781083963_binary` | `include/sak/ranges/operators.hpp`, `include/sak/geometry/point.hpp` |
| `sak::opengl::shader( const string& source, const type shader_type )` compiles in-memory GLSL strings directly, `sak::opengl::program< t_range >` links any `is_shader_range`, and `shader::loader` reads `data/shader/<name>.glsl` files | `include/sak/opengl/shader.hpp`, `include/sak/opengl/program.hpp` |
| `sak::opengl::shader::storage_buffer< t_payload >` enforces `is_flat< t_payload >` (`has_no_padding` and `is_trivially_copyable_v`) and prepends `{ count, stride }` in `float` units | `include/sak/opengl/shader.hpp` |
| SSBO GPU blocks expose `float data[ ];` and reconstruct scalar and vector `float` leaf members, motivating exact integer-valued `float` encoding (`0.0f`, `1.0f`, exact up to $2^{24}$) for `m_type_id` | `docs/agent/ssbo-reflection.md` |
| `include/sak/string.hpp` provides case conversion (`to_lower_case` and `to_upper_case`) only, while `include/sak/format.hpp` wraps `std::vformat` (which parses `{` and `}` delimiters), motivating literal token replacement for GLSL source injection | `include/sak/string.hpp`, `include/sak/format.hpp` |
| `include/sak/math/vector.hpp` currently provides Euclidean vector reductions and Rodrigues `rotate` | `include/sak/math/vector.hpp` |
| Sibling repository `../pickup-express` `.ccf` `0x3100` visual groups store shared indexed vertices per group preserving manifold connectivity alongside per-group `0xf040` / `0xf070` local basis transforms, while `0x1100` stores town collision BSP fragments | `../pickup-express` |
| `sak` module documentation under `docs/agent/sak/cpp/` and `docs/agent/index.md` reflect the current header surface | `docs/agent/sak/cpp/index.md`, `docs/agent/index.md` |

## Session State

- **Valid Reference Implementations**:
  - Stage 0 ([Baseline Reference](analyses/renderer/baseline.md)): `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl`.
  - User hand-written sphere triangle example: `tests/adhoc/rendering/0010_sdl3_demo.cpp` and `data/shader/sdl3-demo.glsl`.
- **Stage 1 ([Point Niebloid Purification](analyses/renderer/point-niebloids.md)) — Complete and Verified**:
  - `project-mcp-tools` commit `30c657d` (`refactor(sak): extract point predicates and product into niebloids`) updated:
    - `geometry/point.hpp`
    - `geometry/geometry.hpp`
    - `math/math.hpp`
  - `draft-point` commit `1931706` (`refactor(sak): adopt extracted point niebloids in geometry, game and tui`) updated:
    - `source/game/game.cpp`
    - `source/tui/terminal.cpp`
    - `source/tui/renderer.cpp`
    - `tests/sak/test_sak_geometry.cpp`
  - `sak::math::length` returns the natural floating-point norm of `square_root( dot( vector ) )` (`sqrt( 14.0 )` for `point< int, 3 >{ 1, 2, 3 }`, asserted within tolerance `1e-6` against `3.7416573867739413` in `tests/sak/test_sak_geometry.cpp`); both `dist/test-sak-geometry` and `dist/test-sak` pass.
- **Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)) — Shifted to Sphere + Trigonometric 3D Triangle (Pending Build/Runtime Verification)**:
  - The user rejected the earlier slab/AABB `cube_figure` / `evaluate_cube` prototype in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` and redirected Stage 2 from sphere + cube to **sphere + trigonometric 3D triangle**.
  - [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) has been updated to specify the CPU `triangle` class (storing absolute vertices in `triangle::data` and delivering precomputed centroid, three centroid-relative vectors, and unit plane normal to the SSBO), `triangle_figure`, and GLSL `evaluate_triangle` (trigonometric ray-plane intersection, sub-triangle oriented-area membership, and `abs( dot( unit_direction, normal ) )` inclination shading).
  - Code implementation and build/runtime verification in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` remain pending.
- **Stages 3 through 7 — Disapproved by User**:
  - Everything planned or authored after Stage 2 remains **disapproved by the user** and held pending revision:
    - Stage 3 Voronoi/AABB/concave-inversion specification (`docs/agent/analyses/renderer/rounded-primitives.md`).
    - All downstream stage designs: `docs/agent/analyses/renderer/adaptive-mesh.md`, `docs/agent/analyses/renderer/orientation.md`, `docs/agent/analyses/renderer/spatial-graph.md`, and `docs/agent/analyses/renderer/pickup-express.md`.
- **Workflow Status — Stage 2 Implementation and Verification**:
  - Active task is implementing and verifying the sphere + trigonometric 3D triangle prototype in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`; downstream Stages 3 through 7 remain held pending revision.
- Documentation separation policy: architecture documents under `docs/agent/analyses/renderer/` record timeless contracts and open questions, while mutable session history and active task state reside in `docs/agent/status.md`.

## Deferred

| Item | Status and Rationale |
|---|---|
| Stage 2 sphere + trigonometric 3D triangle implementation and verification (`tests/adhoc/rendering/0011_agnostic_pipeline.cpp`) | **Pending build/runtime verification**: replace `cube_figure` / slab AABB `evaluate_cube` with CPU `triangle` class (`triangle::data` absolute vertices, precomputed centroid + relative vectors + unit normal), `triangle_figure`, and `evaluate_triangle` |
| Stage 3 Voronoi/AABB/concave-inversion specification (`docs/agent/analyses/renderer/rounded-primitives.md`) | **Disapproved by user**: authored downstream of the prior Stage 2 cube solution; halted and held pending revision |
| Downstream stage designs (`adaptive-mesh.md`, `orientation.md`, `spatial-graph.md`, `pickup-express.md`) | **Disapproved by user**: planned downstream of the prior Stage 2 cube solution; halted and held pending revision |
| Base workflow document (`docs/agent/workflow/index.md`) | Carries an explicit pending note until a shared workflow document location is defined in `project-mcp-tools` |

## Documentation Map

| Document | Subject | Validation Status |
|---|---|---|
| [3D Renderer](analyses/renderer/index.md) | Domain scope, core invariants, and analysis-tree `## Nodes` | Active index; downstream stage status held pending revision |
| [Baseline Reference](analyses/renderer/baseline.md) | Immutable Stage 0 SSBO contract and closest-approach sphere inclination renderer | Valid reference implementation (`source/sdl3_shadertoy.cpp`, `data/shader/sdl3-shadertoy.glsl`) |
| [Point Niebloid Purification](analyses/renderer/point-niebloids.md) | Pure storage `sak::point` class surface and external dimension-symmetric `sak` niebloids | Complete and verified (`30c657d` in `project-mcp-tools`, `1931706` in `draft-point`) |
| [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) | Heterogeneous `is_flat` figure payload, compile-time figure trait registration, token-injected GLSL dispatch, sphere and CPU-precomputed trigonometric 3D triangle evaluators | Architecture updated to sphere + trigonometric 3D triangle; pending build/runtime verification in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` |
| [Rounded Primitives](analyses/renderer/rounded-primitives.md) | Voronoi/AABB convex hybrid `border_radius` cube and interior concave fillet inversion | **Disapproved** AI design; pending revision |
| [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) | Per-corner clamped `border_radius` and dihedral crease rounding on triangle meshes | **Disapproved** AI design; pending revision |
| [Orientation](analyses/renderer/orientation.md) | Dimension-symmetric CPU quaternion algebra reduced to a local-frame `mat3` on GPU | **Disapproved** AI design; pending revision |
| [Spatial Graph](analyses/renderer/spatial-graph.md) | Doubly-linked spatial proximity graph and incremental visible-index discovery | **Disapproved** AI design; pending revision |
| [Legacy Game Preservation](analyses/renderer/pickup-express.md) | Ethical preservation scope and flat dominant-color validation on `.ccf` `0x3100` legacy meshes | **Disapproved** AI design; pending revision |
| [SSBO Reflection](ssbo-reflection.md) | Flat-float GPU storage buffer exchange and C++26 compile-time reflection design | Reference architecture document |
