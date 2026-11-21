# `cpp-install-glad`

Source: [`scripts/cpp-install-glad`](../../../../scripts/cpp-install-glad)

## What it does

Generates the OpenGL 4.6 core profile loader and wraps it in a snake_case C++ header. Four steps:

1. **Generate the loader.** Runs `glad` (via `project-mcp-tools`) for `gl:core=4.6`, no extensions, header-only C, into `include/glad/`.
2. **Create the wrapper class.** `cpp-create-class` creates the header-only class `sak::opengl::glad::snake_case`.
3. **Snake-case the functions.** A Python snippet scans `include/glad/gl.h` for every `#define glXxx glad_glXxx` alias, converts each `glXxx` name to snake_case, and fills the class body with `#define gl_snake_name glad_glXxx` macros.
4. **Relocate.** Moves `include/glad/*` into `include/sak/opengl/glad/`.

## Result

- `include/sak/opengl/glad/gl.h` — the glad loader.
- `include/sak/opengl/glad/snake_case.hpp` — `#define` aliases mapping snake_case names (`gl_bind_texture`) to the glad function pointers (`glad_glBindTexture`).

## Premise for this analysis

The raw C API is already available through glad, flattened into snake_case macros. The analysis starts from this base to map and label every object of the 4.6 core profile and produce a strongly-typed, safe, object-oriented C++ API on top of it.
