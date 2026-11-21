# Tagging

## Decisions

- **`group` semantic tags ingested (from `gl.xml`).** The pipeline labels every function and constant with its `group` semantics from `gl.xml`:
  - group names are converted from the XML's camelCase to **lower-kebab-case** (e.g. `TextureMagFilter` → `texture-mag-filter`);
  - a comma-separated `group` attribute becomes multiple tags;
  - the tags hang under a `group` parent tag (an object linked to `texture-target` inherits `group` via the hierarchy);
  - a function's groups come from the return type (`proto`) and its parameters; a constant's from its `<enum>` `group`, falling back to the enclosing `<enums>` default;
  - gl.xml objects absent from the glad headers (GLX, non-core) are skipped, so only registered objects get tagged.
- **`class` object tags (prefixed `param-*` / `proto-*`).** The `class` attribute of `gl.xml` marks the kind of GL object a handle (GLuint/GLsync) refers to, distinct from `group` (the semantic enum value). A function is tagged with every `class` of its signature, prefixed by where it appears: `param-<class>` from a parameter, `proto-<class>` from the return type (e.g. `glUseProgram` → `param-program`, `glCreateProgram` → `proto-program`, `glTextureBuffer` → `param-buffer` + `param-texture`). A derived `class` root folds `union` over the `class-*` owner tags only (not over `param-*`/`proto-*`): every signature-class member is already a class owner, so the root coincides with the owner scope while remaining semantically correct. The `param-*`/`proto-*` prefix keeps object-class tags in a namespace separate from the `group` tags, resolving the `buffer` collision (a `group` tag `buffer` also exists): `param-buffer` never merges with the `buffer` group. Implemented: `extract.py` (`function_classes`), `pipeline.py` (the prefixed tags and the derived `class` root), `README.md`.
- **`group-` prefix for semantic group tags.** All semantic group tags ingested from `gl.xml` use the `group-` prefix (e.g. `group-texture-mag-filter`, `group-shader-type`) to namespace them clearly from other tag categories. Implemented in `extract.py` and `pipeline.py`.
- **Class ownership semantics.** A function's primary class owner is determined by receiver semantics:
  - `proto-<class>` marks the owner for constructor/creation functions;
  - the first parameter carrying a `class` marks the receiver/owner for operations;
  - any non-first parameter carrying a `class` represents a linked argument/association rather than class ownership (e.g. linking functions belong to the receiver object).
- **Class owner tagging implemented.** Functions are mapped to class owner tags (`class-program`, `class-shader`, `class-buffer`, etc.) based on receiver semantics (constructor return type or first parameter with a class attribute), establishing clean OOP boundaries for C++ API generation. Implemented in `extract.py` and `pipeline.py`.
- **Target-based functions carry no class handle, hence no class owner.** The `class` attribute marks handle parameters; target-based functions (`buffer_data`, `tex_image`, `tex_parameteri`, `vertex_attrib_pointer`) reference the object only via a `target` GLenum, not a handle parameter, so they carry no `param-*`/`proto-*` and no `class-*` owner. Only handle-referencing functions (DSA and bind/gen/attach) get a `class-*` owner. The `class-*` scope is therefore the structural base of the modern object surface. Distinct from the `bind-to-edit` tag, which marks the discouraged *handle-referencing* set excluded from `direct-state-access` (see [Direct State Access](direct-state-access.md)).
- **Enum scoping semantics.** An enum group's scope in the generated C++ API is determined by function receiver cardinality:
  - **Class-scoped (`class::enum`):** when every function consuming that `group` belongs to the same receiver class;
  - **Module-scoped (`sak::opengl::enum`):** when the `group` is consumed across multiple distinct receiver classes or by context/global pipeline functions.
