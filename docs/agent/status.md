# Agent Status

This is the entry document. A session resumes by reading this file only; everything else is reached from the links below.

## Current Goal

- Build a renderer that rounds the sharp edges and corners of legacy 3D games analytically, shaded by surface-ray inclination over flat per-polygon colors, with every step implemented under the `draft-point` dimension-symmetry rule.
- Domain: [3D Renderer](analyses/renderer/index.md). The domain holds the plan, the facts, and the open questions. This file holds only state and the next action.

## Next Task

Implement stage 1 of the roadmap: [Point Niebloid Purification](analyses/renderer/point-niebloids.md).

- Move `is_all`, `is_inside`, `length` and `product` out of the member surface of `sak::point` in `include/sak/geometry/point.hpp` into standalone niebloids, keeping `operator==` as the only operator declared on the class and delegating its body to the extracted predicate.
- `length` is deleted from the class, not reimplemented: `sak::math::length` already is the niebloid.
- No dimension is indexed inside any extracted operation.
- Follow the extraction order in the document: `product`, then `is_all`, then `is_inside` and equality, then the `length` removal.

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

- `include/sak/geometry/point.hpp` declares `operator==` and no other operation of the set `is_all`, `is_inside`, `length`, `product`.
- The extracted niebloids are reachable from `sak` and used by every former call site under `include/`, `source/` and `tests/`.
- Each extracted operation consumes its value as a range; no `[ 0 ]`, `[ 1 ]` or equivalent axis access appears in the implementation.
- `cpp_code_verifier` reports no violations on every changed file.
- `cpp_compile` completes successfully.
- `dist/test-sak-geometry` reports success, with the existing assertions in `tests/sak/test_sak_geometry.cpp` still covering the extracted behaviour.
- The validator/fixer subagent loop required by `docs/agent/style-guide/cpp.md` reaches validator satisfaction, and `git_quick_upload` publishes the result.

## Verified Facts

| Fact | Evidence |
|---|---|
| `hamilton_product` does not exist in `sak` | `rg -L "hamilton\|quaternion" include/ docs/` returned no matches before these documents were written |
| No quaternion type and no matrix type exist in `sak` | same search; `include/sak/math/vector.hpp` offers only Rodrigues `rotate` |
| `sak::point` members are `is_all`, `operator==`, `is_inside`, `length`, `product` | `include/sak/geometry/point.hpp` |
| `point` arithmetic is external, generated in `sak::ranges` by `__781083963_binary` | `include/sak/ranges/operators.hpp`; the `::sak::math::operator+` import list in `point.hpp` |
| `shader` compiles from a source string; `shader::loader` only reads files | `include/sak/opengl/shader.hpp`, `shader::shader`, `shader::loader::load` |
| SSBO payloads must be flat and unpadded, prefixed by `{ count, stride }` | `include/sak/opengl/shader.hpp`, `is_flat`, `storage_buffer` |
| The reference renderer hardcodes one figure type | `data/shader/sdl3-shadertoy.glsl`, the `sphere_ssbo` loop |
| The reference renders a red sphere at `x = -0.55` and a blue sphere at `x = +0.55`, radius `0.45` | `source/sdl3_shadertoy.cpp`, `demo_logic` constructor |
| Sibling preservation project exists at `../pickup-express` | directory listing |
| The `sak` module documents now match the headers | `docs/agent/sak/cpp/index.md`, `geometry.md`, `math.md` and `ranges.md` were rewritten against `include/sak/` listings and the headers they describe |
| `docs/agent/index.md` no longer links a missing `architecture/index.md` | the entry points at `sak/cpp/index.md`, which holds the library philosophy and module map |

## Session State

- No source file was created, changed, or compiled. Every change is markdown.
- Working tree at handoff, in `draft-point`: `docs/agent/analyses/index.md` modified (a pre-existing uncommitted edit by the owner that registers this domain under Analyses), `docs/agent/index.md` modified, `docs/agent/workflow/index.md` modified, `docs/agent/analyses/renderer/` untracked, `docs/agent/status.md` untracked.
- Working tree at handoff, in the sibling `project-mcp-tools`: `docs/agent/sak/cpp/index.md`, `geometry.md`, `math.md` and `ranges.md` modified. The `docs/agent/sak` path is a symlink into that repository, so the shared library documentation is committed there, not here.
- The owner's pre-existing edit to `docs/agent/analyses/index.md` is not to be reverted or rewritten; it is the parent-index registration of this domain.
- Documentation policy, set by the owner: documentation describes the current code, never the history of the code; a module without a document carries a pending note instead of a stale description; no freshness rule and no permission gate exists for updating a document.

## Do Not Re-run

- Do not re-verify the absence of `hamilton_product`; it is answered above and changes only when someone adds it.
- Do not re-derive the inclination brightness of the reference shader; it is recorded in [Baseline Reference](analyses/renderer/baseline.md) with its source.
- Do not re-read the reference shader, `point.hpp`, `shader.hpp` or `operators.hpp` to answer a question already answered in the table above.

## Deferred

| Item | Reason |
|---|---|
| Stages 2 to 7 | Each depends on the previous stage; only stage 1 is actionable now |
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
| [Point Niebloid Purification](analyses/renderer/point-niebloids.md) | Stage 1, and the next task |
| [Agnostic Pipeline](analyses/renderer/agnostic-pipeline.md) | Stage 2: figure contract, injection path, flat payload constraint |
| [Rounded Primitives](analyses/renderer/rounded-primitives.md) | Stage 3: hybrid `border_radius` cube and interior inversion |
| [Adaptive Mesh](analyses/renderer/adaptive-mesh.md) | Stage 4: per-corner clamp, per-edge interpolation, arrow validation |
| [Orientation](analyses/renderer/orientation.md) | Stage 5: quaternion on CPU, `mat3` on GPU |
| [Spatial Graph](analyses/renderer/spatial-graph.md) | Stage 6: doubly-linked graph, camera node, visible set |
| [Legacy Game Preservation](analyses/renderer/pickup-express.md) | Stage 7: ethical scope, flat per-polygon color, visual thesis |
