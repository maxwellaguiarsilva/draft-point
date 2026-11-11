# Agent Status

- Entry-point session document: a session resumes by reading this file first and traversing into linked architecture documents as needed.

## Current Goal

- Build a 3D renderer that rounds sharp edges and sharp corners of legacy 3D games analytically, shaded by surface-ray inclination over flat per-polygon dominant colors, under the `draft-point` dimension-symmetry invariant across C++ and GLSL.
- Domain architecture: [3D Renderer](analyses/renderer/index.md) (`[#current]` node: [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)).

## Next Task

- Implement Stage 3 ([Rounded Primitives](analyses/renderer/rounded-primitives.md)) as an isolated ad-hoc prototype created via `cpp_create_test` (`flg_adhoc = true`, under `tests/adhoc/rendering/`), consuming the Stage 2 upstream contracts now available from [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) and keeping `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` unchanged as the Stage 0 [Baseline Reference](analyses/renderer/baseline.md):
  - **Stage 3a — Convex Hybrid Rounded Box**:
    - Parameterize one box primitive by outer `half_size` and proportional `border_radius` in `[ 0.0, 1.0 ]`: `fillet_radius = half_size * border_radius` and `core = half_size * ( 1.0 - border_radius )`.
    - Classify the Minkowski feature simultaneously across all axes at `entry_point`: `region = sign( entry_point ) * step( vec3( core ), abs( entry_point ) )`, `mask = abs( region )`, `anchor = region * core`.
    - Select the feature through `sum_axis( mask )`: flat core face at `1.0`, cylindrical edge fillet at `2.0`, spherical corner octant at `3.0`.
    - Evaluate the masked subspace quadratic with `direction_norm_squared` division so 3D corners and 2D edges share one discriminant, then shade via `cos_incidence = abs( dot( unit_direction, outer_normal ) )`.
    - Guard convex grazing Voronoi transitions by validating `step( vec3( core ), abs( fillet_point ) ) == mask` and advancing across core-plane slab crossings.
  - **Stage 3b — Half-Space Cut and Interior Concave Fillet Inversion**:
    - Clip with the local half-space plane `( cut_normal, cut_offset )`, retaining `dot( local_point, cut_normal ) <= cut_offset`.
    - Report the retained outer convex hit directly; advance to the far bounding-box slab exit when the front hit is clipped or the ray originates inside the bounding box.
    - Resolve interior curved features through the far quadratic root `concave_distance = ( along + root_span ) / direction_norm_squared` and the backward Voronoi transition guard across corner octant, concave edge cylinder and inner flat wall.
    - Shade the cavity through the inward normal `inner_normal = -outer_normal`, confirming a visible hit whenever `concave_distance > 0.0` and `dot( concave_point, cut_normal ) <= cut_offset`.
  - **Open design choices to resolve** in [Rounded Primitives](analyses/renderer/rounded-primitives.md):
    - Single parameterized rounded-box figure with a sentinel `cut_offset` versus separate convex rounded-box and cut-cavity figure types.
    - Bounded two-step slab advance versus a closed-form analytical rounded-box root solver for convex grazing transitions.
  - **Deferred central question**: the concave fillet inversion formula for the hollow cut cube remains tracked in `## Deferred` as the central mathematical question of Stage 3b.

## Reproduction Commands

| Purpose | Command |
|---|---|
| List renderer analysis documents | `find -L docs/agent/analyses/renderer -type f` |
| Inspect working tree state | `git status --porcelain` |
| Scaffold isolated stage ad-hoc prototype | `cpp_create_test` (`flg_adhoc = true`, under `tests/adhoc/rendering/`) |
| Stage 2 prototype sources | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` and `data/shader/agnostic-pipeline.glsl` |
| Mandatory transitive include fix-up before every compile | `cpp-include-tree --flg-auto-fix` |
| Compile project and ad-hoc binaries | `cpp_compile` |
| Run Stage 2 prototype headless | `xvfb-run dist/0011-agnostic-pipeline` |
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

- Prototype executes as a standalone ad-hoc binary created via `cpp_create_test` (`flg_adhoc = true`) with `//	using	config/sdl3.json` and `#define GLAD_GL_IMPLEMENTATION`, while `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` remain unchanged.
- Convex hybrid rounded box parameterized by outer `half_size` and proportional `border_radius` in `[ 0.0, 1.0 ]`:
  - `border_radius = 0.0` reproduces a sharp cube and `border_radius = 1.0` reproduces an analytical sphere.
  - `fillet_radius = half_size * border_radius` and `core = half_size * ( 1.0 - border_radius )`.
  - Core faces and curved fillets shade continuously, with genuinely curved outer silhouettes at fixed geometry count.
- Feature classification executes as a single multi-axis Voronoi expression: `region = sign( entry_point ) * step( vec3( core ), abs( entry_point ) )`, `mask = abs( region )`, `anchor = region * core`.
- `sum_axis( mask )` selects flat core face (`1.0`), cylindrical edge fillet (`2.0`) and spherical corner octant (`3.0`), and the masked subspace quadratic preserves `direction_norm_squared` division across 3D corners and 2D edges.
- Fillet inclination shades via `cos_incidence = abs( dot( unit_direction, outer_normal ) )`, matching adjacent flat core faces where `outer_normal` aligns with `region`.
- Convex grazing Voronoi transitions validate `step( vec3( core ), abs( fillet_point ) ) == mask` and advance across core-plane slab crossings within a bounded number of transitions.
- Half-space cut plane `( cut_normal, cut_offset )` retains `dot( local_point, cut_normal ) <= cut_offset`, exposing inner flat walls and concave fillets through the clipped opening.
- Interior concave inversion traverses from the far bounding-box slab exit to the far quadratic root `concave_distance = ( along + root_span ) / direction_norm_squared`, with the backward Voronoi transition guard resolving corner octant, concave edge cylinder and inner flat wall monotonically.
- Visible interior hits satisfy `concave_distance > 0.0` and `dot( concave_point, cut_normal ) <= cut_offset`, shading through `inner_normal = -outer_normal` and reporting `hit_distance = concave_distance` for scene depth sorting.
- `cpp-include-tree --flg-auto-fix` and `cpp_compile` succeed cleanly.
- Mandatory validator/fixer sub-agent loop (`docs/agent/style-guide/all.md` and `docs/agent/style-guide/cpp.md`) reaches validator satisfaction prior to `git_quick_upload`.

## Verified Facts

| Fact | Evidence |
|---|---|
| `sak::point` is pure coordinate storage declaring only `operator ==` and six constructors, including prefix-range + trailing scalars `point( range, first, remaining... )` | `include/sak/geometry/point.hpp` |
| `sak::is_all`, `sak::is_inside_of`, and `sak::is_equal` are free niebloids in flat `namespace sak`; `sak::math::length` and `sak::math::product` are free reduction niebloids in `sak::math` | `include/sak/geometry/point.hpp`, `include/sak/math/vector.hpp`, `include/sak/math/math.hpp` |
| `point` element-wise binary and compound-assignment arithmetic operators are generated externally in `sak::ranges` by `__781083963_binary` | `include/sak/ranges/operators.hpp`, `include/sak/geometry/point.hpp` |
| `sak::opengl::shader( const string& source, const type shader_type )` compiles in-memory GLSL strings directly, `sak::opengl::program< t_range >` links any `is_shader_range`, and `shader::loader` reads `data/shader/<name>.glsl` files | `include/sak/opengl/shader.hpp`, `include/sak/opengl/program.hpp` |
| `sak::opengl::shader::storage_buffer< t_payload >` enforces `is_flat< t_payload >` (`has_no_padding` and `is_trivially_copyable_v`) and prepends `{ count, stride }` in `float` units | `include/sak/opengl/shader.hpp` |
| SSBO GPU blocks expose `float data[ ];` and reconstruct scalar and vector `float` leaf members, motivating exact integer-valued `float` encoding (`0.0f`, `1.0f`, exact up to $2^{24}$) for `m_type_id` | `docs/agent/ssbo-reflection.md` |
| `include/sak/string.hpp` provides case conversion (`to_lower_case` and `to_upper_case`) only, while `include/sak/format.hpp` wraps `std::vformat` (which parses `{` and `}` delimiters), motivating literal token replacement for GLSL source injection | `include/sak/string.hpp`, `include/sak/format.hpp` |
| Baseline reference renderer hardcodes `sphere_ssbo` with a red sphere at `x = -0.55f` and a blue sphere at `x = +0.55f` (`y = 0.0f`, `z = 1.0f`, `radius = 0.45f`), using unnormalized `direction` and sphere-center `length( hypotenuse )` depth comparison | `source/sdl3_shadertoy.cpp`, `data/shader/sdl3-shadertoy.glsl`, [Baseline Reference](analyses/renderer/baseline.md) |
| Stage 2 ad-hoc prototype lives in `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` with its standalone fragment template in `data/shader/agnostic-pipeline.glsl`, while `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` remain the unchanged Stage 0 baseline | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`, `data/shader/agnostic-pipeline.glsl`, [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) |
| Registered figures are compile-time trait types collected in one `constexpr std::tuple` (`sphere_figure`, `cube_figure`), and a single `static_assert` over that tuple validates every `m_parameter_count` against the shared `parameter_capacity` | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` |
| The CPU generates `switch( uint( figure_item.type_id ) )` with one `case` per registered trait and injects it at the `//	@figure_dispatch` marker | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`, `data/shader/agnostic-pipeline.glsl` |
| The fragment template carries three placeholder tokens (`//	@figure_parameter_capacity`, `//	@figure_functions`, `//	@figure_dispatch`), each restored with `#line <marker_line + 1>` computed relative to the fragment section whose first line is `#version` | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`, `data/shader/agnostic-pipeline.glsl` |
| `cpp-include-tree --flg-auto-fix` and `cpp_compile` succeed, and `dist/0011-agnostic-pipeline` runs headless under `xvfb-run` long enough to prove shader compilation and program linking, with a captured frame showing the red sphere at `x = -0.55f` beside the blue slab cube at `x = +0.55f` | `tests/adhoc/rendering/0011_agnostic_pipeline.cpp`, `dist/0011-agnostic-pipeline` |
| `include/sak/math/vector.hpp` currently provides Euclidean vector reductions and Rodrigues `rotate`, while dimension-symmetric `hamilton_product`, `quaternion_rotate`, and `orientation_basis` are specified in [Orientation](analyses/renderer/orientation.md) over 3D `cross` and `dot`, `point( imaginary_part, scalar_part )`, and cyclically `rotated` basis vectors | `include/sak/geometry/point.hpp`, `include/sak/math/vector.hpp`, [Orientation](analyses/renderer/orientation.md) |
| Sibling repository `../pickup-express` `.ccf` `0x3100` visual groups store shared indexed vertices per group preserving manifold connectivity alongside per-group `0xf040` / `0xf070` local basis transforms, while `0x1100` stores town collision BSP fragments | `../pickup-express` |
| `sak` module documentation under `docs/agent/sak/cpp/` and `docs/agent/index.md` reflect the current header surface | `docs/agent/sak/cpp/index.md`, `docs/agent/index.md` |

## Session State

- Stage 1 ([Point Niebloid Purification](analyses/renderer/point-niebloids.md)) is complete, verified, and published:
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
- Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)) is complete and validated:
  - Added `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` and `data/shader/agnostic-pipeline.glsl`.
  - Resolved the three design choices as compile-time figure trait types in one `constexpr std::tuple`, a CPU-generated `switch( uint( figure_item.type_id ) )` dispatch, and a standalone `.glsl` template read through `sak::fso::text_file`.
  - `sak::opengl::shader::loader`, `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` are untouched.
  - The mandatory validator/fixer loop reported `SATISFIED`, and `dist/0011-agnostic-pipeline` renders the red sphere and blue slab cube headless under `xvfb-run`.
- Documentation separation policy: architecture documents under `docs/agent/analyses/renderer/` record timeless contracts and open questions, while mutable session history and active task state reside in `docs/agent/status.md`.

## Deferred

| Item | Status and Rationale |
|---|---|
| Stages 3 through 7 | Stage 3 ([Rounded Primitives](analyses/renderer/rounded-primitives.md)) is next after the completed Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)); Stages 5 and 6 become parallelizable once the Stage 4 ([Adaptive Mesh](analyses/renderer/adaptive-mesh.md)) contracts are in place |
| Quaternion niebloids (`hamilton_product`, `quaternion_rotate`, `orientation_basis`) in `sak` | Dimension-symmetric formulation verified over 3D `cross` and `dot`, `point( imaginary_part, scalar_part )`, and `quaternion_rotate` across cyclically `rotated` basis vectors; scheduled for Stage 5 ([Orientation](analyses/renderer/orientation.md)) |
| `mat3` flat-float transfer convention | Open design choice recorded in [Orientation](analyses/renderer/orientation.md) |
| Concave fillet inversion formula for the hollow cut cube | Central mathematical question of Stage 3b in [Rounded Primitives](analyses/renderer/rounded-primitives.md) |
| Mesh edge-adjacency representation on CPU and GPU | Central data-layout question in [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) |
| 3D frustum coverage from spatial proximity links | Open algorithmic question in [Spatial Graph](analyses/renderer/spatial-graph.md) |
| Per-polygon dominant-color extraction metric | Open asset-processing question in [Legacy Game Preservation](analyses/renderer/pickup-express.md) |
| Base workflow document (`docs/agent/workflow/index.md`) | Carries an explicit pending note until a shared workflow document location is defined in `project-mcp-tools` |

## Documentation Map

| Document | Subject |
|---|---|
| [3D Renderer](analyses/renderer/index.md) | Domain scope, core invariants, and analysis-tree `## Nodes` (`[#current]`) |
| [Baseline Reference](analyses/renderer/baseline.md) | Immutable Stage 0 SSBO contract and closest-approach sphere inclination renderer |
| [Point Niebloid Purification](analyses/renderer/point-niebloids.md) | Pure storage `sak::point` class surface and external dimension-symmetric `sak` niebloids |
| [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) | Heterogeneous `is_flat` figure payload, compile-time figure trait registration, token-injected GLSL dispatch, and unified sphere/slab-cube evaluators |
| [Rounded Primitives](analyses/renderer/rounded-primitives.md) | Convex hybrid `border_radius` cube and interior concave fillet inversion |
| [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) | Per-corner clamped `border_radius` and dihedral crease rounding on triangle meshes |
| [Orientation](analyses/renderer/orientation.md) | Dimension-symmetric CPU quaternion algebra reduced to a local-frame `mat3` on GPU |
| [Spatial Graph](analyses/renderer/spatial-graph.md) | Doubly-linked spatial proximity graph and incremental visible-index discovery |
| [Legacy Game Preservation](analyses/renderer/pickup-express.md) | Ethical preservation scope and flat dominant-color validation on `.ccf` `0x3100` legacy meshes |
| [SSBO Reflection](ssbo-reflection.md) | Flat-float GPU storage buffer exchange and C++26 compile-time reflection design |
