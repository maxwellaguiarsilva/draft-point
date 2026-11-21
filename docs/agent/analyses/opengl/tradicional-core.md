# Traditional Core

## Objective

`traditional-core` is the standard bind-to-edit Core functionality that remains functional but is inferior for modern OOP design.

## Decision

- **`traditional-core` = the bind-to-edit set.** The inferior tier is the bind-to-edit operations excluded from `direct-state-access` (the closed set: the `gen_*`/`framebuffer_texture*`/`framebuffer_renderbuffer`/`tex_buffer*` prefixes plus the target/`location` binds rejected by `is_modern_bind`; draw/capture pipeline binds — single-handle `vertex-array`/`program-pipeline` and target `framebuffer`/`transform-feedback` — stay in the surface). Their explicit labeling is deferred; they are the complement of the DSA surface within their classes.

## Nodes

- `bind-to-edit` — the bind-to-edit operation set complementing the DSA surface.
