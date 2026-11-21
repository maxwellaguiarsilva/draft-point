# Draw execution

## Objective

Define the `draw-execution` operation group: the pipeline-driven draw execution commands that contribute to the derived `core-pipeline` union, and from there to `canonical`. Separate the commands that dispatch rendering from those that configure state or capture data.

## Decision

- **`draw-execution` = dispatch of rendering, not configuration.** A draw-execution command executes the drawing itself (assembles primitives and rasterizes them) from already-configured objects; it does not set state, bind targets, or configure objects.
- **No structural marking exists in `gl.xml`.** Commands carry no operation-level tag that distinguishes draw-execution; the XML classifies types, bitmasks and parameter groups, but never the semantics of a command's operation. Classification is domain judgment over the operation, consistent with the undocumented-legacy decision in [`tiers.md`](tiers.md).
- **Signature evidence: `GLenum mode` with `group="PrimitiveType"`.** The first parameter of every draw command is a `mode` selecting the primitive type to assemble/rasterize (`PrimitiveType`). This is a strong, queryable heuristic — it is already materialized in the pipeline as the `group-primitive-type` tag.
- **The `PrimitiveType` heuristic is near-exclusive after the Core filter.** The pipeline already filters to the Core profile, which removes the extension variants and the immediate-mode `gl_begin`. The only remaining non-draw command carrying `PrimitiveType` in the current database is `gl_begin_transform_feedback` (capture, not drawing). Treating that single case is the remaining scope decision.
- **No more modern replacement; stays `canonical`.** Under the modernity tie-breaker, none of the draw-execution commands has a more modern Khronos recommendation performing the same operation, so they pass as `canonical` via the `core-pipeline` union.
- **Signature subgroups are derivable, not the operation.** Presence of `instancecount`/`drawcount` and the `BufferOffset` indirect kind sub-classify within the group, but never by themselves identify the group.

## Open scope questions

- **Compute dispatch inclusion.** Whether `gl_dispatch_compute` / `gl_dispatch_compute_indirect` join `draw-execution` (execution dispatch, though not drawing) or form a separate group.
- **Transform-feedback draw inclusion.** Whether `gl_draw_transform_feedback*` belong to the group (they dispatch drawing, but derive vertex count implicitly from a transform-feedback object).
- **`gl_begin_transform_feedback` treatment.** The single residual false-positive for the `PrimitiveType` heuristic; whether excluded by rule or by enumeration.

## Implementation

Not yet materialized. The `draw-execution` tag does not exist in the pipeline; `group-primitive-type` (the heuristic evidence) is already derived in `opengl-tag`.
