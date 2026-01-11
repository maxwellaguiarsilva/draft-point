# Workflow: Backlog Recovery Protocol (Technical Diary)

This guide describes the mechanics for recovering and documenting the technical evolution of the project through git commits, ensuring historical accuracy and organization.

## 1. Fundamental Principles

### The Golden Rule: "One Day at a Time"
To avoid context overload and loss of detail, the agent must strictly process **a single day per chat session**. 
- If the user requests an interval (e.g., 01/01 to 10/01), the agent processes day 01, updates the progress, and concludes its part.
- Progress is tracked via `docs/analyses/head.md`.
- It is strictly forbidden to analyze more than one day's per chat session.
- If you do this, your task will be marked as failed and this behavioral failure will be recorded.

### Separation of Concerns
This workflow is purely documental. No changes to the source code (`source/` or `include/`) should be made during the execution of this protocol.

---

## 2. Daily Cycle Step-by-Step

1.  **Read State:** Read `docs/analyses/head.md` to find the next pending date.
2.  **History Analysis:** Use `git log` and `git show` to analyze the changes for the specific day.
3.  **Report Generation:** Create the `docs/analyses/YYYY-MM-DD.md` file in `en-us` using the template defined in `head.md`.
4.  **Checklist Update:** Mark the date as completed `[x]` in the `head.md` file.
5.  **Statistic Registration:** Call the `agent_statistic` tool with `name="success"`.

---

## 3. Consolidation Phase and Final Delivery

This phase occurs only when **all** days of the requested period have been processed individually.

### Step A: Blind Concatenation (Via Shell)
Instead of manually reading and summarizing, the agent must use the terminal to create a single consolidated file in `en-us`. This ensures that no technical information is omitted.

```bash
# Example for a specific period
cat docs/analyses/YYYY-MM-*.md > docs/analyses/YYYYMM_merged_en.md
```

### Step B: Cleaning Temporary Artifacts
After concatenation, the individual daily files must be removed to avoid filesystem clutter.

```bash
rm docs/analyses/YYYY-MM-*.md
```

### Step C: Translation and Delivery in PT-BR
The agent must read the resulting `merged_en.md` file and perform a **simple and direct translation** to Brazilian Portuguese, saving the result to:
`docs/analyses/pt-br/YYYYMM_daily_note.md`

### Step D: Final Cleanup
Remove the consolidated English file (`merged_en.md`) and the `docs/analyses/head.md` file, leaving only the final report in Portuguese in the `pt-br/` directory.

---

## 4. Success Verification
The workflow is considered successful when:
1. The `head.md` file reflects that all days have been processed.
2. The consolidated document in `pt-br/` exists and contains the translated complete history.
3. No individual `.md` date files remain in the `docs/analyses/` directory.
