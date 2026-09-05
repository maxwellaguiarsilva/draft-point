# Data model

## Decisions

- **Flat `tag` + `rule` + `operation`, no hierarchy.** `parent_id` is removed — it expresses the same thing a derived tag does, but rigidly (single parent, tree) and with less flexibility. The remodel:
  - `tag(id, name)` — the tag entity; no operation column.
  - `object_tag(object_id, tag_id)` — the only physical marking, orthogonal and mandatory per object.
  - `operation(id_operation, name)` — the operation catalog: `union`, `intersection`, `difference`.
  - `rule(position, id_tag, id_operation, id_target)` — composite primary key `(position, id_tag)`. Each row applies one operation (`id_operation`) to one target tag (`id_target`); the tag's membership is an ordered fold over its rows by `position`, so operations can be mixed.
  - Every "level above" (the `group` root, the tier tree, bitmask sets) becomes a derived tag defined by rules, computed on read, never stored.
  - **Rationale.**
    - Tags mark orthogonal facts directly on members; any aggregation is a derived view.
    - This avoids per-object repetition for derived concepts, keeps the operation catalog DRY (each operator name lives once), and removes the fixed-hierarchy constraint.
    - **Example.** `bitmask-groups` folds `union` over the `type="bitmask"` group tags, then `bitmask` folds `intersection` over `constants` and `bitmask-groups`.
    - **Supersedes** the earlier `parent_id`-based decisions:
      - `indirect tier labeling`;
      - `draft hierarchy tree`.
  - **Implemented.**
    - schema;
    - `db.py` (ordered-fold membership);
    - `pipeline.py` (derived `group` root);
    - `README.md` rewritten to the flat model.
- **Bitmask sets derived (`bitmask-groups` + `bitmask`).** `type="bitmask"` lives only on the `<enums>` blocks of `gl.xml`. The pipeline derives two tags:
  - `bitmask-groups` — `union` over the bitmask group tags, folding only groups that already exist as tags (vendor-only bitmask groups with no registered object are skipped, so no empty tag is created);
  - `bitmask` — `intersection` over `constants` and `bitmask-groups`, isolating the constants that belong to a bitmask group (functions referencing a bitmask group stay in `bitmask-groups` but not in `bitmask`).
  Rationale: bitmask membership is a derived view over the group tags, not a physical marking; the two-level fold mirrors the `group` root pattern and keeps `bitmask` scoped to constants only. Implemented: `extract.py` (`bitmask_group_names`), `pipeline.py` (the two derived tags), `README.md`.
