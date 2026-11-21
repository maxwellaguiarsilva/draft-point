# Direct State Access

## Objective

Derive the `direct-state-access` tag as the modern object surface: everything a `class` owner exposes, minus the bind-to-edit operations. Then derive the `dsa-*` and RAII tiers from it.

## Decision

- **Base: the `class` owner scope (structural).** The `class` root is a derived tag folding `union` over the `class-*` owner tags only (not over `param-*`/`proto-*`, which add no members). It is the structural "operates on this object by handle" set.
- **Subtract the bind-to-edit set (closed, rule-derived).** `direct-state-access` is a derived tag: the `class` owner scope minus the bind-to-edit set. The bind-to-edit set is closed and identified **entirely by rules** (no name enumeration): the clean name prefixes `gen_*`, `framebuffer_texture*`/`framebuffer_renderbuffer`, `tex_buffer*`, plus the `bind_*` family filtered by `is_modern_bind`.
- **`is_modern_bind` (signature rule, not tokens).** A `gl_bind_*` function is excluded (bind-to-edit) unless it binds to a unit/index slot. Predicates over the `gl.xml` signature:
  - `flg_location` — a `location` token in the name (the GLSL `layout(location=...)` qualifier is the modern replacement);
  - `flg_index` — an `index`/`first` parameter (indexed or multi bind);
  - `flg_enum_target` — a `GLenum target` first parameter;
  - `flg_target_bind` — `flg_enum_target` without an index slot.
  - **`is_modern_bind = not(flg_location or flg_target_bind)`.** Modern unit/index/multi binds stay in the surface; target/locator binds are excluded.
  - **Single-handle binds are not bind-to-edit.** A `gl_bind_*` with a single handle parameter makes an object current for pipeline use, not bind-to-edit, and has no modern substitute; it stays in the surface. A removed `flg_context_bind` (single-handle) rule wrongly relegated these.
  - **Draw/capture-pipeline bind classes have no modern substitute.** A `gl_bind_*` whose class owner is a render/capture pipeline class is mandatory to render into / capture with; no DSA or unit-bind replaces it, so it stays in the surface despite carrying a `GLenum target` first parameter. Applies to `framebuffer` and `transform-feedback`:
    - `bind_framebuffer`;
    - `bind_transform_feedback`.
  - **Modern unit/index/multi binds stay in the surface.**
    - `bind_buffer_base`;
    - `bind_buffers_base`;
    - `bind_texture_unit`;
    - `bind_textures`;
    - `bind_image_texture`;
    - `bind_sampler`.
  - **Single-handle draw-pipeline binds stay in the surface** (no modern substitute):
    - `bind_vertex_array`;
    - `bind_program_pipeline`.
- **Derive `dsa-*` by intersection.** Each `dsa-<class>` = `intersection(direct-state-access, class-<class>)`.
- **Derive the RAII tiers last**, as unions over the `dsa-*` sub-tags:
  - `dsa-single-raii` — union of `dsa-shader`, `dsa-program`;
  - `dsa-list-raii` — union of `dsa-buffer`, `dsa-texture`, `dsa-framebuffer`, `dsa-vertex-array`, `dsa-renderbuffer`, `dsa-sampler`, `dsa-transform-feedback`, `dsa-query`.

## Rationale

- **Subtraction beats addition.** The bind-to-edit set is closed and identifiable, while the modern verb set is open-ended (new verbs keep appearing; `get_*` introspection was missed by the name-token heuristic).
- **Signature beats a name-token heuristic for binds.** The `bind_*` prefix alone over-matches the modern bind-to-use API; the `gl.xml` signature (unit/index/multi slot vs target/context/locator) separates the families without enumeration. `bind_sampler` is kept alongside `bind_texture_unit` (same unit-bind shape); `bind_buffers_base`/`bind_buffers_range` are kept (indexed multi binds with a `GLenum target` first param).
- **Current-bind with no modern substitute is `canonical`, not bind-to-edit.** A bind whose sole purpose is to make an object current for the render/capture pipeline has no DSA or GLSL replacement and must stay in the surface. This applies to the single-handle `bind_vertex_array` / `bind_program_pipeline` and to the target-bound `bind_framebuffer` / `bind_transform_feedback` (their `GLenum target` is a degenerate singleton current-slot, not an edit target). A prior single-handle-param (`flg_context_bind`) predicate mislabeled the former, and a naive target-bind predicate mislabels the latter; the exemption is by class owner, consistent with the modernity tie-breaker.
- **"discouraged" ≈ "bind-to-edit".** Excluding bind-to-edit excludes the discouraged legacy layer Khronos never formally deprecated in `gl.xml`, including the `location`-qualifier locator binds.
- **Lifecycle and inherently-direct classes are kept.** `create`/`delete`/`is` (the RAII surface) and inherently-direct classes (`shader`, `program`, `sampler`, `sync`, `query`) are not bind-to-edit, so they stay in `direct-state-access`.

## Implementation

Implemented in `opengl-tag`: `extract.py` (`function_param_signatures`), `pipeline.py` (`_is_modern_bind`, `_bind_to_edit_names`, physical `bind-to-edit` tag, derived `direct-state-access` tag, `dsa-*` intersections, RAII tier unions). `direct-state-access` = `class` minus `bind-to-edit`; each `dsa-<class>` = intersection of `direct-state-access` with `class-<class>`; `dsa-single-raii`/`dsa-list-raii` = unions over the `dsa-*` sub-tags. `_is_modern_bind` excludes the `flg_context_bind` predicate, and `_bind_to_edit_names` additionally exempts the draw/capture pipeline bind classes (`framebuffer`, `transform-feedback`) via `BIND_PIPELINE_CLASSES`.
