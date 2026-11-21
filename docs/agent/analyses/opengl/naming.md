# Naming

## Decisions

- **Convention: snake_case.** The generated C++ API uses snake_case (e.g. `gl_bind_texture`) instead of the original camelCase of the GL API. Matches the already-generated `snake_case.hpp` wrapper and the project's `sak` style.
- **Original name on the object row.** The `object` table carries an `original_name` column instead of a separate translation table:
  - `NULL` when the snake_case wrapper did not alter the name — constants and types keep their API name, so `name` already equals the original;
  - for functions it stores the camelCase source.
  Rationale: the original name is a property of a single object, so a nullable column on the row keeps the schema simpler than a second table, and `NULL` communicates "unchanged" without duplicating values.
- **Atomic tags without composite names.** Compound tag names (e.g. using delimiters like `:`, prefixing, or suffixing) are prohibited. Orthogonal concepts must be modeled as separate, pure atomic tags rather than compound strings, preserving granular queries without substring matching. Direct multi-tagging of objects and recursive parent-tag hierarchies in the database schema provide complete direct and indirect domain control.
- **Snake_case vector-type-suffix tokenization resolved upstream in `to_snake`.** Vector-type suffixes (`v`, `iv`, `fv`, `dv`, `bv`, `sv`, `ubv`, `uiv`, `usv`, `iiv`, `iuiv`) are separated cleanly by fixing `to_snake` in `scripts/cpp-install-glad` upstream. Rationale: resolving tokenization at the generation source produces clean snake_case identifiers throughout the generated wrapper and downstream tagging pipeline without requiring post-hoc normalizations.
  Implementation detail: the suffix is matched case-insensitively at the end of the name so variant prefixes (`I`, `L`, `N`) keep their own token, and `dv` is guarded with `(?<!e)` so the double-vector suffix never steals the `d` of a past-tense `-ed` verb (`glScissorIndexedv` stays `gl_scissor_indexed_v`, `glGetVertexAttribdv` stays `gl_get_vertex_attrib_dv`). Re-running `cpp-install-glad` regenerated `snake_case.hpp` (e.g. `gl_get_sync_iv`, `gl_clear_buffer_fv`), with no name collisions.
