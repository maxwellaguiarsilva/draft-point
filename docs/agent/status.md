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


