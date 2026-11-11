# Agent Status

- Entry-point session document: a session resumes by reading this file first and traversing into linked architecture documents as needed.

## Current Goal

- Build a 3D renderer that rounds sharp edges and sharp corners of legacy 3D games analytically, shaded by surface-ray inclination over flat per-polygon dominant colors, under the `draft-point` dimension-symmetry invariant across C++ and GLSL.
- Domain architecture: [3D Renderer](analyses/renderer/index.md) (`[#current]` node: [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)).

## Next Task

- Implement Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)) as an isolated ad-hoc prototype created via `cpp_create_test` (`flg_adhoc = true`, under `tests/adhoc/rendering/`), keeping `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` unchanged as the Stage 0 [Baseline Reference](analyses/renderer/baseline.md):
  - **Stage 2a — CPU Figure Contract and Token-Injected Shader Assembly**:
    - Define the fixed-stride `is_flat` figure payload with registration-time slot capacity validation:
      - `m_position` (`sak::point< float, 3 >`)
      - `m_type_id` (exact integer-valued `float`)
      - `m_color` (`sak::point< float, 4 >`)
      - `m_parameters` (`std::array< float, t_parameter_count >`)
    - Assemble the fragment shader on the CPU using named placeholder token replacement (`//	@figure_functions` and `//	@figure_dispatch`), `#line` restoration directives, and the shared GLSL dimension-symmetry prelude:
      - `rotated_1`
      - `rotated_2`
      - `min_axis`
      - `max_axis`
      - `sum_axis`
    - Compile shaders in memory via `sak::opengl::shader( source, type )` and link via `sak::opengl::program`, keeping `sak::opengl::shader::loader` file-based and unmodified.
  - **Stage 2b — Heterogeneous Sphere + Cube Scene**:
    - Populate a single `storage_buffer` with one sphere on the left (`x = -0.55f`) and one axis-aligned slab cube on the right (`x = +0.55f`, `y = 0.0f`, `z = 1.0f`).
    - Evaluate both primitives through the injected GPU dispatch:
      - Normalize `unit_direction = normalize( direction )` once per fragment.
      - Shade surfaces via `ambient + volume * cos_incidence`.
      - Sort front hits by world-metric surface `hit_distance`.
  - Resolve the three Stage 2 open design choices documented in [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md):
    - C++ compile-time figure trait types vs runtime descriptor registration.
    - `switch( uint( figure_item.type_id ) )` vs `if` / `else if` GLSL dispatch.
    - Standalone `.glsl` fragment template file vs embedded raw string literal.

## Reproduction Commands

| Purpose | Command |
|---|---|
| List renderer analysis documents | `find -L docs/agent/analyses/renderer -type f` |
| Inspect working tree state | `git status --porcelain` |
| Scaffold isolated Stage 2 ad-hoc prototype | `cpp_create_test` (`flg_adhoc = true`) |
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

- Prototype executes as a standalone ad-hoc binary created via `cpp_create_test` (`flg_adhoc = true`) with `//	using	config/sdl3.json` and `#define GLAD_GL_IMPLEMENTATION`, while `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl` remain unchanged.
- Fragment shader outer loop stays independent of concrete figure geometry:
  - Normalizes `unit_direction` once per fragment.
  - Iterates `figure_ssbo.count` and dispatches by `uint( figure_item.type_id )`.
  - Sorts hits by world-metric surface `hit_distance > 0.0`.
  - Shades the winning hit via `ambient + volume * cos_incidence`.
- Sphere and axis-aligned slab cube evaluators express all coordinate math through whole-vector GLSL intrinsics and the shared prelude helpers, confining axis-component swizzles to the single fragment prelude.
- CPU injects evaluator functions and dispatch via named placeholder replacement and `#line` directives into `sak::opengl::shader( source, type )`; `sak::opengl::shader::loader` retains its file-loading interface.
- Heterogeneous figure payload satisfies `is_flat`, stores `m_type_id` as an exact integer-valued `float`, and validates that every registered figure fits within `m_parameters` capacity.
- Interactive scene renders one sphere at `x = -0.55f` and one cube at `x = +0.55f`, both shaded analytically by ray-surface inclination with distinct front and side plane brightness on the cube.
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
- Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)) is marked `[#current]` in [3D Renderer](analyses/renderer/index.md) with premeditated CPU/GLSL contracts ready for Stage 2a and Stage 2b implementation.
- Documentation separation policy: architecture documents under `docs/agent/analyses/renderer/` record timeless contracts and open questions, while mutable session history and active task state reside in `docs/agent/status.md`.

## Deferred

| Item | Status and Rationale |
|---|---|
| Stages 3 through 7 | Sequenced after Stage 2 ([Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md)); Stages 5 and 6 become parallelizable once Stage 2 and Stage 4 contracts are in place |
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
| [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) | Heterogeneous `is_flat` figure payload, token-injected GLSL dispatch, and unified sphere/slab-cube evaluators |
| [Rounded Primitives](analyses/renderer/rounded-primitives.md) | Convex hybrid `border_radius` cube and interior concave fillet inversion |
| [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) | Per-corner clamped `border_radius` and dihedral crease rounding on triangle meshes |
| [Orientation](analyses/renderer/orientation.md) | Dimension-symmetric CPU quaternion algebra reduced to a local-frame `mat3` on GPU |
| [Spatial Graph](analyses/renderer/spatial-graph.md) | Doubly-linked spatial proximity graph and incremental visible-index discovery |
| [Legacy Game Preservation](analyses/renderer/pickup-express.md) | Ethical preservation scope and flat dominant-color validation on `.ccf` `0x3100` legacy meshes |
| [SSBO Reflection](ssbo-reflection.md) | Flat-float GPU storage buffer exchange and C++26 compile-time reflection design |
