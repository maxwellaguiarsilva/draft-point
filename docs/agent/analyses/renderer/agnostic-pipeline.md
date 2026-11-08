# Agnostic Pipeline

Import: [3D Renderer](index.md)

## Scope

- Stage 2: decouple the CPU scene description from the geometric nature of each figure, and let the CPU assemble the fragment shader that evaluates all figures.
- Success replaces the hardcoded sphere loop of [Baseline Reference](baseline.md) with a dispatch over a generic object list, while the visual result stays comparable: one sphere on the left, one cube on the right, both shaded by inclination.

## Facts

### Injection Is Already Possible

- `sak::opengl::shader` has a public constructor that compiles a source string directly: `shader( const string& source, const type shader_type = vertex )`.
- `sak::opengl::shader::loader` only accepts a file name, reads `data/shader/<name>.glsl`, splits combined files on marker lines of the form `//\t<kind>-shader`, and caches by name. It has no in-memory entry point.
- `program` accepts any range whose reference converts to `const shader&`, so a CPU-built collection of shaders can be fed to it without the loader.
- Evidence: `include/sak/opengl/shader.hpp`, `shader::shader`, `shader::loader::load`, the `is_shader_range` concept.

- Consequence: the placeholder injection path does not need a new loader API. Either the template stays a file read through `sak::fso::text_file` and is transformed into source strings before construction, or a loader-side source hook is added; the first requires no change to `sak`.

### Payload Constraint

- `storage_buffer< t_payload >` asserts `is_flat< t_payload >`, defined as no padding between consecutive members plus trivially copyable, and prefixes `{ count, stride }` where `stride` is the payload size in floats.
- A heterogeneous object list must therefore be a fixed-stride flat struct. The payload for every figure type shares one layout and one size; figure-specific values occupy a fixed slot region.
- Evidence: `include/sak/opengl/shader.hpp`, `has_no_padding`, `is_flat`, `storage_buffer`.
- Consequence: the abstract figure describes how many floats it consumes and which offsets it reads; the buffer stride is the maximum over the registered figures. A figure larger than the slot region fails at registration, not at render time.

### Figure Contract

- The CPU base type owns what every figure has: `position`, `color`, orientation later in [Orientation](orientation.md), and the figure type identifier.
- Each concrete figure answers two questions:
  - Which data does this figure need beyond the base? For a sphere, only `radius`.
  - Which GLSL source evaluates this figure's ray intersection and its inclination brightness? The answer is a string.
- The CPU concatenates the figure bodies into the fragment shader placeholder and emits the dispatch that maps a type identifier to the matching body.
- The generic object carries the type identifier and the data; the GPU loop selects the formula by identifier.

### Dispatch Shape

- The per-figure function keeps the [Baseline Reference](baseline.md) contract: it receives the ray origin and direction plus the figure payload, and returns the hit distance and the brightness, so the outer loop only compares distances.
- The outer loop stays identical for every figure count, including zero, because the buffer count already drives it.

## Exclusions

- The shader template format, the placeholder names, and the exact type-identifier encoding are not decided here.
- No figure other than sphere and cube is registered in this stage; the rounded cube is stage 3.
- Nothing in this stage changes the camera, the environment buffer, or the screen projection.

## Open Questions

- Whether the dispatch is an `if`/`else if` chain, a `switch`, or a function-pointer table. The chain is the simplest and is inlined per figure; the table costs an indirect call per pixel per object.
- Whether the generated source is written to a temporary file so the existing loader stays the only source reader, or whether `shader::table` is built in memory. The in-memory path keeps the tree clean but bypasses the cache and the file-name diagnostics.
- Whether the figure list is assembled from C++ types (one class per figure, each contributing both payload semantics and GLSL source) or from a table of strings. The type-driven form keeps the payload and the formula from diverging; the string form is faster to prototype.

## Relations

- Consumes the inclination formula and buffer contract of [Baseline Reference](baseline.md).
- Assumes the niebloids produced by [Point Niebloid Purification](point-niebloids.md), because the figure payloads are built from `sak::point` operations.
- Produces the dispatch used by [Rounded Primitives](rounded-primitives.md) for the hybrid figure.
- Feeds [Adaptive Mesh](adaptive-mesh.md), which registers a mesh figure in the same way.
