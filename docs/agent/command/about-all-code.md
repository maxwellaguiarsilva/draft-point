---
description: Attach the whole project C++ source and header tree to the conversation so the user can discuss the entire codebase.
---

Attach every C++ source and header of the project to the conversation so the user can ask about the codebase as a whole.

1. **List the files:** run the following from the project root:
   ```bash
   find -L source include -type f \
     \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' -o -name '*.cc' \) \
     -not -name 'gl.h' -not -name 'snake_case.hpp' -not -path '*/expand64/*'
   ```
   The exclusions are mandatory: `gl.h` (generated GLAD loader), `snake_case.hpp` and the `expand64/` generators together account for roughly 77% of the tree's tokens and must never be attached.
2. **Attach each file:** read every file returned by the find command with the `read` tool, issuing the reads in parallel batches. Do not skip or truncate any file.
3. **Report:** state only the number of files attached. Do not dump file contents into the chat.

User text: `$ARGUMENTS`
