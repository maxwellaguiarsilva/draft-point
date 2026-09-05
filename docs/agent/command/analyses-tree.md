---
description: It helps the user to work on the analysis of a subject that is documented in the form of a subject tree.
agent: build
model: deepseek/deepseek-v4-flash
---

**Rules:**
1. Entry point: `docs/agent/analyses/index.md`. It is the root of the analysis tree. Each document can only have a single level-tree; sub-items must be in a separate document and nodes point to them. If a node is very complex, it may have its own folder with `index.md` containing its nodes, otherwise, it will simply be a new markdown file in the same folder. You can notice the difference by the link referencing the document (`node/index.md` vs `node.md`).
2. Each document has a section called "Nodes" which contains the list of topics at that level. The current node must be marked with [#current]. **Empirically validate the item marked [#current] has already been resolved:** inspect the actual state to confirm whether the marked item is still open. If it has already been resolved, report it and ask the user if he authorizes going to the next item in the current level-tree. If in the current level-tree no node is marked then the first one should be marked.
3. Register information as structured bullet lists or trees, not prose paragraphs. Inline lists are prohibited — each item must be its own list bullet or tree node. It is not permitted to record:
    - Comment about the sub-items of its items. This information should be in the item's specific document.
    - Which items are open or closed. Which is the current item, the next item, or the previous item. This control is done exclusively by the [#current] marker. The question before it is assumed to be closed, and the one after it is assumed to be open.
    - Numbers, and names are not part of the analysis rules. The scalar result and the data produced should only be output from the control tools and never recorded in markdown.
4. If you find any violation of this agreement, stop everything you are doing and report it to the user.
5. **Mechanics of identifying the current node:** start at the entry point (`docs/agent/analyses/index.md`). Read its Nodes section, locate the node marked `[#current]`, and open the document it points to. Repeat this recursively in each opened document: locate the `[#current]` node and follow it to its document. Stop when the current node's document has no Nodes section or its current node points to no further document — that is the deepest current node (a leaf). Report the analysis status of that leaf with a simple and objective summary.
6. **Report** only the files changed and a one-line summary of the decision recorded.
7. **Archive an analysis on explicit request.** If the user asks to archive an analysis, move its documents from `docs/agent/analyses/<analysis>/` into the archive. The archive is organized by year: move the documents into `docs/agent/analyses/.archived/<year>/<yyyyMMdd>-<analysis>/`. Remove the entry from `docs/agent/analyses/index.md`.

User text: `$ARGUMENTS`

