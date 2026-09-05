---
description: Uploads all related Git repositories (draft-point and its sibling project-mcp-tools) by pulling, committing changes grouped by topic, and pushing to their remotes.
agent: build
model: deepseek/deepseek-v4-flash
---

This workspace spans two Git repositories:

- **`draft-point`**: the main project (the current working directory).
- **`project-mcp-tools`**: a sibling repository (`../project-mcp-tools`) holding shared tooling and library sources.

Part of `draft-point`'s tree is made of symlinks pointing into `project-mcp-tools` (e.g. `include/sak`, `scripts`). Files reached through those symlinks belong to `project-mcp-tools`, so their changes are committed from inside that repository.

**Procedure:** run the upload workflow below in `draft-point`, then in `project-mcp-tools`, then return to `draft-point`.

1. **Fetch Latest Code:** run `git pull` in the current repository.
2. **Analyze Changes:** inspect `git status` and `git diff` to understand what was modified, including untracked files.
3. **Handle Untracked Files:** stage all relevant untracked files; rely on `.gitignore` to exclude build artifacts (`__pycache__/`, `.venv/`, etc.).
4. **Commit Changes Grouped by Topic:** analyze the changes and group them by distinct topics. It is strictly forbidden to commit unrelated changes in the same commit. To ensure proper separation:
   - **Categorize:** classify the changes into themes: *Feature* (new logic), *Fix* (bug correction), *Refactor* (code cleanup), *Style* (formatting/spacing), *Docs* (markdown/comments), *Chore* (configs/dependencies).
   - **Group Files:** assign each modified file to its theme. Files changed for the exact same reason (e.g. a class and its unit test) belong in the same group.
   - **Stage and Commit:** for each group, stage only those files and commit them. Never use `git add .` or `git commit -a` when there are multiple themes. If a commit message contains backticks (`), escape them with a backslash (\).
   Do not ask for approval for the commit message; proceed automatically.
5. **Push:** run `git push`.
6. **Final Validation:** confirm `git status` is 100% clean (no modified, staged, or untracked files).
7. **Move to the Next Repository:** if the workflow just ran in `draft-point`, switch to `project-mcp-tools` and repeat steps 1-6 there; then return to `draft-point`. Skip this step if the workflow ran only in `project-mcp-tools`.
8. **Communication:** report that the procedure completed successfully, showing exactly which commit message(s) were used, each enclosed in backticks.

User text: `$ARGUMENTS`
