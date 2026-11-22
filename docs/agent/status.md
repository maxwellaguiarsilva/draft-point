# Agent Status

- Entry-point session document: a session resumes by reading this file first and traversing into linked architecture documents as needed.

## Current Goal

- Render the figures demo in `source/figures_demo.cpp` with `data/shader/figures.glsl` and the figure evaluators under `data/shader/figures/`.

## Next Task

- awaiting the user's next iteration

## Reproduction Commands

| Purpose | Command |
|---|---|
| Inspect working tree state | `git status --porcelain` |
| Mandatory transitive include fix-up before every compile | `cpp-include-tree --flg-auto-fix` |
| Compile the project | `cpp_compile` |
| Verify C++ formatting rules on changed `.hpp` / `.cpp` files | `cpp_code_verifier` |
| Publish validated changes | `git_quick_upload` |

## Verified Facts

| Fact | Evidence |
|---|---|
| `sak::opengl::shader` compiles in-memory GLSL strings, `shader::split_sections` parses a combined document into `sections`, `shader::compile( const sections& )` builds the shader table, and the filesystem is a consumer choice ( `text_file` reads the document in the demo ) | `include/sak/opengl/shader.hpp`, `source/figures_demo.cpp` |
| `shader::storage_buffer< t_payload >` requires `is_flat` ( `has_no_padding` and `is_trivially_copyable_v` ) and prefixes `{ count, stride }` in `float` units | `include/sak/opengl/shader.hpp`, `docs/agent/ssbo-reflection.md` |
| SSBO GPU blocks expose `float data[ ];` and rebuild scalar and vector members from consecutive floats, so `m_type_id` travels as an exact integer-valued `float` ( exact up to $2^{24}$ ) | `docs/agent/ssbo-reflection.md` |
| Each figure file is one braced block that writes the shared dispatch `result`, and the dispatcher computes `inclination = abs( dot( direction, best_hit.hit_normal ) )` once after the traversal | `data/shader/figures.glsl`, `data/shader/figures/sphere.glsl`, `data/shader/figures/triangle.glsl`, `source/figures_demo.cpp` |
| `include/sak/string.hpp` provides case conversion only, and `include/sak/format.hpp` wraps `std::vformat`, which parses `{` and `}` delimiters, motivating literal token replacement for GLSL source injection | `include/sak/string.hpp`, `include/sak/format.hpp` |

## Documentation Map

| Document | Subject |
|---|---|
| [SSBO layout](ssbo-reflection.md) | Flat-float GPU storage buffer exchange and the `is_flat` payload contract |
| [sak library](sak/cpp/index.md) | Module map of `include/sak/` |
