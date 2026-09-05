# opengl-tag

Mini `uv` project that backs the `opengl` analysis. It maintains a SQLite database (`opengl_tag.db`) tagging the objects of the OpenGL 4.6 core profile extracted from the generated glad headers.

This project follows **no** project documentation rules; it exists only to support the analysis.

## Pipeline

At the start of the pipeline, if `gl.xml` is not present locally, it is downloaded via `curl` (the source URL lives in `pipeline.py`) — this is the first stage executed, before any extraction.

`opengl-tag` then extracts OpenGL API objects from two glad headers and registers them in the database:

| source | object | tag |
|---|---|---|
| `snake_case.hpp` | functions `gl_*` | `functions` |
| `gl.h` | constants `GL_*` | `constants` |
| `gl.h` | types `GL*` (typedefs) | `types` |
| `gl.xml` | functions & constants | `group` semantics (prefixed `group-*`) |
| `gl.xml` | functions | `class` object tags (prefixed `param-*` / `proto-*`) and owner tags (`class-*`) |
| `gl.xml` | bitmask `<enums>` blocks | derived `bitmask-groups` / `bitmask` |
| `snake_case.hpp` + `gl.xml` | functions | `bind-to-edit` (physical, rule-derived) |

Each object becomes an `object` row linked to its category tag. Functions and constants are additionally tagged with their `group` semantics from `gl.xml`: the camelCase `group` attribute(s) are converted to lower-kebab-case with a `group-` prefix (e.g. `TextureMagFilter` → `group-texture-mag-filter`) and linked to `group` tags; the `group` root is a derived tag (the union of every group tag). A function's groups come from its return type and parameters; a constant's from its `<enum>` `group`, falling back to the enclosing `<enums>` default. gl.xml objects absent from the glad headers are skipped. The pipeline is **idempotent** (re-running adds nothing). The `--reset` flag recreates the database from scratch.

Bitmask sets are derived from the `type="bitmask"` `<enums>` blocks: `bitmask-groups` is the union of every bitmask group tag, and `bitmask` is the intersection of `constants` with `bitmask-groups` (the constants that belong to a bitmask group). Vendor-only bitmask groups without a registered object are skipped (no empty tag is created).

Functions are additionally tagged with their `class` object tags and class owner tags: the `class` attribute of `gl.xml` marks the kind of GL object a handle refers to. Every `class` of a function's signature becomes a tag prefixed by where it appears — `param-<class>` from a parameter, `proto-<class>` from the return type. In addition, each function is linked to its primary receiver/constructor class owner tag (`class-<class>`). A derived `class` root folds the union of the `class-*` owner tags only (the `param-*`/`proto-*` tags are excluded: every signature-class member is already a class owner, so the root equals the owner scope). The `param-*`/`proto-*` and `class-*` prefixes keep object-class tags separate from the `group` semantic tags (resolving the `buffer` name collision).

Functions are additionally tagged `bind-to-edit` (the set Khronos discourages in favor of a more modern approach), derived entirely from rules — never enumerated. Clean name prefixes (`gen_*`, `framebuffer_texture*`/`framebuffer_renderbuffer`, `tex_buffer*`) mark whole families; the ambiguous `bind_*` family is split by `is_modern_bind`, a predicate over the `gl.xml` signature (unit/index/multi binds stay, target/`location` binds are excluded, single-handle draw-pipeline binds stay). Target binds of the draw/capture pipeline classes `framebuffer` and `transform-feedback` are additionally exempted via `BIND_PIPELINE_CLASSES` (their current bind is mandatory and has no modern substitute). The `direct-state-access` tag is then derived as the `class` owner scope minus `bind-to-edit`.

Types excluded as non-API: the glad loader internals (`GLAD*`, `GLVULKANPROCNV`), the per-function `PFNGL` pointers (covered by `functions`) and the EGL-interop aliases (`GLeglClientBufferEXT`, `GLeglImageOES`).

## Usage

```sh
uv run opengl-tag               # register incrementally
uv run opengl-tag --reset       # recreate database and register from scratch
uv run opengl-tag --count-by-tag  # print objects per tag and exit
uv run opengl-tag --tag <name>  # print objects belonging to a tag and exit
```

Optional overrides: `--header <snake_case path>`, `--gl-header <gl.h path>`, `--db <path>` and `--gl-xml <gl.xml path>`.

## Schema

- `object (id, name, original_name)` — the tagged OpenGL objects. `name` is the snake_case name; `original_name` holds the original camelCase API name when the two differ.
- `tag (id, name)` — the tag entity.
- `object_tag (object_id, tag_id)` — the many-to-many links (the only physical marking).
- `operation (id_operation, name)` — the operation catalog: `union`, `intersection`, `difference`.
- `rule (position, id_tag, id_operation, id_target)` — composite primary key `(position, id_tag)`. Each row applies one operation to one target tag; a tag's membership is an ordered left fold over its rows by `position`.

Convention: `original_name` is `NULL` when `name` was not altered by the `snake_case.hpp` wrapper (constants `GL_*` and types `GL*` keep their original name), meaning `name` **is** the original API name. For functions, `name` is snake_case (`gl_bind_texture`) and `original_name` stores the camelCase source (`glBindTexture`).

A tag with no rules is physical (its members are the `object_tag` links). A tag with rules is derived, computed on read and never stored: an ordered left fold seeded with the identity of the first operation (`union` → empty, `intersection`/`difference` → every object). Reading an object's tags returns every tag whose membership contains it (see `tag_database.tags_of`). The number of objects per tag, most populated first, is available via `tag_database.count_by_tag`.

## Python `sak` dependency

The Python `sak` library is available in this project through a symlink at `src/sak` (pointing to `project-mcp-tools/sak-lib/python/sak`). Consume it with a plain `import sak`; use it from that path, never from the original project.

## Modules

- `models.py` — domain records (`Rule`, `TagCount`, `ObjectRecord`).
- `engine.py` — `TagEngine` algebraic evaluator with memoization and cycle detection.
- `extract.py` — reads the headers and returns the distinct function, constant and type names.
- `pipeline.py` — orchestrates registration (`run`).
- `db.py` — the `tag_database` sqlite layer.
