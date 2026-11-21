# Tooling

## Decisions

- **Python tooling via `uv`.** No loose scripts folder; all Python work lives in the `uv` project `opengl-tag/`.
- **Tagging in SQLite.** Object labels are stored in a SQLite database (`opengl_tag.db`):
  - an object holds N tags;
  - a tag holds N tags (`parent_id`);
  - reading an object's tags returns the direct tags plus all ancestors, recursively.
  Schema and access live in `opengl_tag/db.py`.
- **Incremental automated pipeline.** `opengl-tag` extracts the objects automatically from the glad headers via an idempotent pipeline (`--reset` recreates the DB). No manual per-object tagging.
- **Three category tags.** Objects are registered under three root tags:
  - `functions` — from `snake_case.hpp`;
  - `constants` — `GL_*`, from `gl.h`;
  - `types` — `GL*` typedefs, from `gl.h`.
  Excluded as non-API: the glad loader internals (`GLAD*`), the per-function `PFNGL` pointers (covered by `functions`) and the EGL-interop aliases.
- **Base tag `functions`.** Every object extracted from `snake_case.hpp` is a GL function; all are registered and tagged `functions` (verified: every `gl_*` macro maps 1:1 to a `glad_gl*` function pointer in `gl.h`, none is a constant or enum).
- **`gl.xml` is the labeling source.** The `glad2` package bundles the Khronos `gl.xml`, which already carries a fragmented but complete labeling structure:
  - **Object kind** — top-level sections `<types>` / `<enums>` / `<commands>` (matches the current three tags).
  - **Semantics** — the `group` attribute.
  - **Bitmask sets** — `type="bitmask"`.
  - **API scope** — the `api` attribute.
  - **Version and core-vs-compat** — `<feature>` `<require>` / `<remove profile="core">`.
  The extracted constants are essentially all core (only a handful fall in the `<remove profile="core">` blocks). This structure is the basis for the next item: the project's own labeling scheme.
- **`gl.xml` is complete.** Empirically validated: every object extracted from the glad headers (functions, constants, types) is present in `gl.xml`; there are no orphans, so the XML can serve as the single source of truth for labeling.
