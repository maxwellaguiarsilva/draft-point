---
description: Realign columns with tabs on specified lines of a file using show-lines and calculate-column-tabulations.
---

Format and align columns using tabs on specified lines of a file. `$1` is the file name to search for and `$2` is the line range specification.

1. **Confirm inputs:** if `$1` or `$2` is missing, ask the user for the missing information before proceeding.
2. **Locate the file:** run `find -L . -name "*$1*"` in the current working directory to find matching files.
3. **Handle multiple matches:** if more than one file is found, list all the results and ask the user which path to use; wait for the answer before continuing.
4. **Calculate column tabulations:** execute `show-lines <path> $2 | calculate-column-tabulations | cat -A` to inspect the tab characters displayed as `^I`. If multiple line ranges were provided, process each range sequentially.
5. **Apply changes:** read the target file with the read tool, then replace the original lines in `<path>` with the generated tab-aligned lines using the edit tool.
6. **Report:** confirm which file was modified and which line ranges were aligned.

User text: `$ARGUMENTS`
