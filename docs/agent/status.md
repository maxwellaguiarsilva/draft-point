# Agent Status

- Entry-point session document: a session resumes by reading this file first and traversing into linked architecture documents as needed.

## Current Goal

- Develop the renderer directly in the demo prototypes under `tests/adhoc/rendering/` and in the shader sources under `data/shader/`.
- The staged renderer plan and its analysis documents were discontinued and removed from `docs/agent/analyses/`.

## Next Task

- awaiting the user's next demo iteration

## Reproduction Commands

| Purpose | Command |
|---|---|
| Inspect working tree state | `git status --porcelain` |
| Valid reference sources (Stage 0 and user triangle-sphere demo) | `data/shader/sdl3-shadertoy.glsl`, `tests/adhoc/rendering/0010_sdl3_demo.cpp`, and `data/shader/sdl3-demo.glsl` |
| Scaffold isolated ad-hoc prototype | `cpp_create_test` (`flg_adhoc = true`, under `tests/adhoc/rendering/`) |
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

## Verified Facts

| Fact | Evidence |
|---|---|
| Stage 0 baseline renderer (`sphere_ssbo` with two spheres using unnormalized `direction` and sphere-center `length( hypotenuse )` depth comparison) remains a valid reference implementation | `data/shader/sdl3-shadertoy.glsl` |
| User's hand-written sphere triangle example remains a valid reference implementation for the single-triangle ray intersection and inclination-color strategy | `tests/adhoc/rendering/0010_sdl3_demo.cpp`, `data/shader/sdl3-demo.glsl` |
| `sak::point` is pure coordinate storage declaring only `operator ==` and six constructors, including prefix-range + trailing scalars `point( range, first, remaining... )` | `include/sak/geometry/point.hpp` |
| `sak::is_all`, `sak::is_inside_of`, and `sak::is_equal` are free niebloids in flat `namespace sak`; `sak::math::length` and `sak::math::product` are free reduction niebloids in `sak::math` | `include/sak/geometry/point.hpp`, `include/sak/math/vector.hpp`, `include/sak/math/math.hpp` |
| `point` element-wise binary and compound-assignment arithmetic operators are generated externally in `sak::ranges` by `__781083963_binary` | `include/sak/ranges/operators.hpp`, `include/sak/geometry/point.hpp` |
| `sak::opengl::shader( const string& source, const type shader_type )` compiles in-memory GLSL strings directly, `sak::opengl::program< t_range >` links any `is_shader_range`, and `shader::loader` reads `data/shader/<name>.glsl` files | `include/sak/opengl/shader.hpp`, `include/sak/opengl/program.hpp` |
| `sak::opengl::shader::storage_buffer< t_payload >` enforces `is_flat< t_payload >` (`has_no_padding` and `is_trivially_copyable_v`) and prepends `{ count, stride }` in `float` units | `include/sak/opengl/shader.hpp` |
| SSBO GPU blocks expose `float data[ ];` and reconstruct scalar and vector `float` leaf members, motivating exact integer-valued `float` encoding (`0.0f`, `1.0f`, exact up to $2^{24}$) for `m_type_id` | `docs/agent/ssbo-reflection.md` |
| Each figure file is one braced block that writes the shared dispatch `result`, and the dispatcher derives `inclination = abs( dot( direction, best_hit.hit_normal ) )` once after the traversal from the saved closest hit | `data/shader/agnostic-pipeline.glsl`, `data/shader/figures/sphere.glsl`, `data/shader/figures/triangle.glsl`, `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` |
| `include/sak/string.hpp` provides case conversion (`to_lower_case` and `to_upper_case`) only, while `include/sak/format.hpp` wraps `std::vformat` (which parses `{` and `}` delimiters), motivating literal token replacement for GLSL source injection | `include/sak/string.hpp`, `include/sak/format.hpp` |
| `include/sak/math/vector.hpp` currently provides Euclidean vector reductions and Rodrigues `rotate` | `include/sak/math/vector.hpp` |
| Sibling repository `../pickup-express` `.ccf` `0x3100` visual groups store shared indexed vertices per group preserving manifold connectivity alongside per-group `0xf040` / `0xf070` local basis transforms, while `0x1100` stores town collision BSP fragments | `../pickup-express` |
| `sak` module documentation under `docs/agent/sak/cpp/` and `docs/agent/index.md` reflect the current header surface | `docs/agent/sak/cpp/index.md`, `docs/agent/index.md` |

## Session State

- **Valid Reference Implementations**:
  - Stage 0: `data/shader/sdl3-shadertoy.glsl`.
  - User hand-written sphere triangle example: `tests/adhoc/rendering/0010_sdl3_demo.cpp` and `data/shader/sdl3-demo.glsl`.
- **Active demo**: `tests/adhoc/rendering/0011_agnostic_pipeline.cpp` with `data/shader/agnostic-pipeline.glsl` and the figure blocks under `data/shader/figures/`.
- **Discontinued plan**: the staged renderer roadmap and its analysis documents were removed; development continues directly in the demos.
- **Point niebloids**: `sak::point` is pure coordinate storage and its predicates and product live as free niebloids (`30c657d` in `project-mcp-tools`, `1931706` in `draft-point`).

## Deferred

| Item | Status and Rationale |
|---|---|
| Base workflow document (`docs/agent/workflow/index.md`) | Carries an explicit pending note until a shared workflow document location is defined in `project-mcp-tools` |

## Documentation Map

| Document | Subject | Validation Status |
|---|---|---|
| [SSBO Reflection](ssbo-reflection.md) | Flat-float GPU storage buffer exchange and C++26 compile-time reflection design | Reference architecture document |
