# Canonical

## Objective

`canonical` is the modern, state-of-the-art Core profile tier: Direct State Access and modern canonical names, derived as a union over its concept-group children.

## Decision

- **`canonical` is a derived union (target), physical labeling removed.** `canonical` = union of `direct-state-access` and `core-pipeline`. It is not a physical name-token tag. The name-token heuristic that previously materialized `canonical` as a physical tag was **removed from the pipeline**; converting `canonical` to the derived union is the next step of this analysis. The `immutable-storage` group was eliminated: under the modernity tie-breaker, every immutable-storage operation either already has a DSA form (captured by `direct-state-access`) or is a bind-to-edit flavor superseded by DSA, so it contributes no canonical members beyond DSA.

## Nodes

- [Direct State Access](direct-state-access.md) [#current] — deriving `direct-state-access` (class-owner scope minus bind-to-edit) and the `dsa-*`/RAII tiers.
- [core-pipeline](core-pipeline.md) — pipeline-driven operation groups.
