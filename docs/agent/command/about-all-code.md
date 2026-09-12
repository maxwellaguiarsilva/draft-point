---
description: Attach the whole project C++ source and header tree to the conversation so the user can discuss the entire codebase.
---

Attach every C++ source and header of the project to the conversation so the user can ask about the codebase as a whole.

1. **Dump the files:** run the following from the project root. It lists, sorts, and prints every file's content with a path header in a single command, so no individual `read` calls are needed:
   ```bash
   find -L source include -type f \
     \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' -o -name '*.cc' \) \
     -not -name 'gl.h' -not -name 'snake_case.hpp' -not -path '*/expand64/*' \
     -print0 | sort -z | xargs -0 -n1 sh -c 'printf "\n===== %s =====\n" "$1"; cat "$1"' _
   ```
   The exclusions are mandatory: `gl.h` (generated GLAD loader), `snake_case.hpp` and the `expand64/` generators together account for roughly 77% of the tree's tokens and must never be attached.
2. **Report:** state only the number of files attached (count the `=====` path headers). Do not dump file contents into the chat.

User text: `$ARGUMENTS`
