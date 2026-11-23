# Agent Status

- Entry-point session document: a session resumes by reading this file first and traversing into linked architecture documents as needed.
- The current subject is documented in **[Figure Demo](figure-demo/index.md)**: read its index first, then the nodes that index links.

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


