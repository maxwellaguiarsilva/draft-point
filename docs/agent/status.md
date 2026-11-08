# Agent Status

This is the entry document. A session resumes by reading this file only; everything else is reached from the links below.

## Current Goal

- Build a renderer that rounds the sharp edges and corners of legacy 3D games analytically, shaded by surface-ray inclination over flat per-polygon colors, with every step implemented under the `draft-point` dimension-symmetry rule.
- Domain: [3D Renderer](analyses/renderer/index.md). The domain holds the plan, the facts, and the open questions. This file holds only state and the next action.

## Next Task

Implement stage 2 of the roadmap: [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md).

- Replace the hardcoded sphere loop in `data/shader/sdl3-shadertoy.glsl` with a dispatch over a generic flat figure list.
- Build the fragment source on the CPU and inject it through the existing `shader( source, type )` constructor; `shader::loader` keeps its file-only API.
- Keep the buffer payload flat and fixed-stride, with figure-specific slots, and reject at registration a figure whose slots exceed the stride.
- Render one sphere on the left and one cube on the right, both shaded by inclination.
- Resolve the three open questions of the stage document: the dispatch form, the source-injection route (temporary file versus in-memory), and whether figures are C++ types or a table of strings.

## Reproduction Commands

| Purpose | Command |
|---|---|
| Confirm the domain documents exist | `find -L docs/agent/analyses/renderer -type f` |
| Confirm working tree state | `git status --porcelain` |
| Mandatory include fix-up before any compile | `cpp-include-tree --flg-auto-fix` |
| Compile the project | `cpp_compile` |
| Verify formatting of changed files | `cpp_code_verifier` on each changed `.hpp`/`.cpp` |
| Run the geometry test binary after a successful compile | `dist/test-sak-geometry` |
| Publish the validated change | `git_quick_upload` |

- `cpp-include-tree`, `cpp_compile`, `cpp_code_verifier` and `git_quick_upload` are tools of the `project-mcp-tools` MCP server; their catalog and conventions live in `../project-mcp-tools/README.md`.
- The `cpp-include-tree --flg-auto-fix` run is mandatory before every compile regardless of what the change looks like.

## Acceptance Criteria for the Next Task

- No figure-specific loop remains in the fragment shader; the outer loop iterates the payload count and selects the formula by type identifier.
- The CPU assembles the dispatch and the generated fragment source; `shader::loader` keeps its current API and only reads files.
- The payload satisfies `is_flat` and keeps one fixed stride for every figure; registration rejects a figure whose slots exceed that stride.
- One sphere renders on the left and one cube on the right, both shaded by inclination.
- `cpp_compile` completes successfully.
- The validator/fixer subagent loop required by `docs/agent/style-guide/cpp.md` reaches validator satisfaction, and `git_quick_upload` publishes the result.

## Verified Facts

| Fact | Evidence |
|---|---|
| `hamilton_product` does not exist in `sak` | `rg -L "hamilton\|quaternion" include/ docs/` returned no matches before these documents were written |
| No quaternion type and no matrix type exist in `sak` | same search; `include/sak/math/vector.hpp` offers only Rodrigues `rotate` |
| `point` declares only `operator ==`; `is_all`, `is_inside_of` and `is_equal` are free niebloids in flat namespace `sak`; `length` and `product` are `sak::math` niebloids | `include/sak/geometry/point.hpp`, `__is_all`, `__is_inside_of`, `__is_equal`; `include/sak/math/vector.hpp`, `__length`; `include/sak/math/math.hpp`, `__product` |
| `point` arithmetic is external, generated in `sak::ranges` by `__781083963_binary` | `include/sak/ranges/operators.hpp`; the `::sak::math::operator+` import list in `point.hpp` |
| `shader` compiles from a source string; `shader::loader` only reads files | `include/sak/opengl/shader.hpp`, `shader::shader`, `shader::loader::load` |
| SSBO payloads must be flat and unpadded, prefixed by `{ count, stride }` | `include/sak/opengl/shader.hpp`, `is_flat`, `storage_buffer` |
| The reference renderer hardcodes one figure type | `data/shader/sdl3-shadertoy.glsl`, the `sphere_ssbo` loop |
| The reference renders a red sphere at `x = -0.55` and a blue sphere at `x = +0.55`, radius `0.45` | `source/sdl3_shadertoy.cpp`, `demo_logic` constructor |
| Sibling preservation project exists at `../pickup-express` | directory listing |
| The `sak` module documents describe the current headers, with one recorded drift in `geometry.md` | `docs/agent/sak/cpp/index.md`, `geometry.md`, `math.md` and `ranges.md`; the drift note in [Point Niebloid Purification](analyses/renderer/point-niebloids.md) |
| `docs/agent/index.md` no longer links a missing `architecture/index.md` | the entry points at `sak/cpp/index.md`, which holds the library philosophy and module map |

## Session State

- Stage 1 is implemented, built, tested and published; no stage-2 code exists yet.
- Commits: `project-mcp-tools` `30c657d` `refactor(sak): extract point predicates and product into niebloids`; `draft-point` `1931706` `refactor(sak): adopt extracted point niebloids in geometry, game and tui`.
- Three files changed in `project-mcp-tools`, under `sak-lib/cpp/include/sak/`: `geometry/point.hpp`, `geometry/geometry.hpp`, `math/math.hpp`.
- Four files changed in `draft-point`: `source/game/game.cpp`, `source/tui/terminal.cpp`, `source/tui/renderer.cpp`, `tests/sak/test_sak_geometry.cpp`.
- Evidence: `cpp_include_tree --flg_auto_fix` reported no redundant includes; `cpp_compile` succeeded; `dist/test-sak-geometry` and `dist/test-sak` pass.
- Naming decisions: `is_inside_of` because `geometry::rectangle::is_inside` shadows the bare name; `is_equal` because a public `equal` would compete with `std::equal_to`; the predicates live in flat `namespace sak` because the class template `sak::geometry` already occupies that name.
- `length` truncation: deleting the member removed its implicit `t_scalar` return conversion, so `sak::math::length` yields the natural norm, `sqrt( 14 )` for `point< int, 3 >{ 1, 2, 3 }`. The owner approved the assertion change to `close( length( p ), 3.7416573867739413, 1e-6 )`, and `source/game/game.cpp` prints the length as a decimal.
- Both repositories were clean when stage 1 was published.
- Documentation policy, set by the owner: documentation describes the current code, never the history of the code; a module without a document carries a pending note instead of a stale description; no freshness rule and no permission gate exists for updating a document.

## Do Not Re-run

- Do not re-verify the absence of `hamilton_product`; it is answered above and changes only when someone adds it.
- Do not re-derive the inclination brightness of the reference shader; it is recorded in [Baseline Reference](analyses/renderer/baseline.md) with its source.
- Do not re-read the reference shader, `point.hpp`, `shader.hpp` or `operators.hpp` to answer a question already answered in the table above.

## Deferred

| Item | Reason |
|---|---|
| Stages 3 to 7 | Each depends on the previous stage; only stage 2 is actionable now |
| `hamilton_product` implementation in `sak` | Needs a symmetric formulation attempt before adding a component-wise fallback; not needed until stage 5 |
| `mat3` transfer convention over flat floats | Undecided; recorded in [Orientation](analyses/renderer/orientation.md) |
| Concave inversion math for the cut cube | The central open question of [Rounded Primitives](analyses/renderer/rounded-primitives.md) |
| Mesh adjacency representation | The central open question of [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) |
| Frustum coverage by two boundary pointers | Explicitly immature in [Spatial Graph](analyses/renderer/spatial-graph.md) |
| Dominant-color definition per polygon | Open in [Legacy Game Preservation](analyses/renderer/pickup-express.md) |
| Base workflow document for `draft-point` | `project-mcp-tools` has no workflow document, so `docs/agent/workflow/index.md` carries a pending note until a replacement location is defined |

## Documentation Map

| Document | Subject |
|---|---|
| [3D Renderer](analyses/renderer/index.md) | Domain index, invariants, stage order, document roles |
| [Baseline Reference](analyses/renderer/baseline.md) | Buffer contract, payload layouts, inclination formula of the current renderer |
| [Point Niebloid Purification](analyses/renderer/point-niebloids.md) | Stage 1, implemented |
| [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) | Stage 2: figure contract, injection path, flat payload constraint |
| [Rounded Primitives](analyses/renderer/rounded-primitives.md) | Stage 3: hybrid `border_radius` cube and interior inversion |
| [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) | Stage 4: per-corner clamp, per-edge interpolation, arrow validation |
| [Orientation](analyses/renderer/orientation.md) | Stage 5: quaternion on CPU, `mat3` on GPU |
| [Spatial Graph](analyses/renderer/spatial-graph.md) | Stage 6: doubly-linked graph, camera node, visible set |
| [Legacy Game Preservation](analyses/renderer/pickup-express.md) | Stage 7: ethical scope, flat per-polygon color, visual thesis |
