# SSBO Reflection

Status: planned. No phase has started.

## Objective

Generate, from C++ reflection, the GLSL declarations that expose the CPU-side payload structs to the fragment shader through shader storage buffers (SSBOs). The authored fragment code contains only `main` and the helpers it needs. Binding indices and every generated declaration are derived automatically.

## Demo-locality

This is an experiment that lives entirely in the demo. Every piece of code it adds or changes stays in `source/sdl3_demo.cpp` and in the fragment section of `data/shader/shadertoy.glsl`. There is no current plan to move any part of it to `sak` or to any other module.

## Related documents

- [C++ style guide](style-guide/cpp.md), which imports [common style rules](style-guide/all.md)
- [OpenGL architecture](analyses/opengl/architecture.md)
- [Buffer storage and updates](analyses/opengl/buffer-storage.md)

## Required reading

Read these before editing:

- `docs/agent/style-guide/cpp.md` and `docs/agent/style-guide/all.md`
- `include/sak/using.hpp`
- `include/sak/sak.hpp`
- `../project-mcp-tools/README.md`, relative to the project root, because validation uses its tools

## Prohibited access

- Do not enter `docs/agent/analyses/opengl/opengl-tag/`. The OpenGL index prohibits access without reading its README first, and this task does not need that folder.
- Scope every search to an explicit path. Filter `.venv` and `__pycache__` directories.
- Do not modify `../project-mcp-tools`.

## Scope

In scope:

- `source/sdl3_demo.cpp`: the payload structs, the payload registry, `shader_storage_buffer`, `shader_loader`, `polygon`, and the buffer construction sites.
- `data/shader/shadertoy.glsl`: the fragment section only. The vertex section is not modified.

Out of scope:

- `sak` and every other module. No file outside `source/sdl3_demo.cpp` and the fragment section of `data/shader/shadertoy.glsl` is touched.
- The `resolution` uniform and its literal location `1`, which `window_listener::pixel_resize` also sets by literal. It stays hand-synced.
- The `final_color` output.
- Payload members that are not floats.

## Current state

Verified from the code and docs:

- `shader_loader::load` reads `data/shader/<name>.glsl`, splits sections on the marker lines `//\t<kind>-shader`, and builds one `shader( body, kind )` per section.
- `shader` takes `( const string& source, const type shader_type = vertex )`. The `type` enumerators are `vertex`, `fragment`, `geometry`, `tess_control`, `tess_evaluation`, and `compute`. Compile and link failures throw `runtime_error` with the driver info log.
- `shader_storage_buffer<t_payload>( count )` creates a buffer, writes a header `{ count, stride }` where stride is the payload size in floats, and uploads the payload with `flush()`. A separate `bind( binding_index )` call makes the buffer visible to the shader.
- `is_flat` is `has_no_padding` plus `is_trivially_copyable`. It does not check element types.
- Binding numbers are typed by hand on both sides. GLSL uses `binding = 0` for environment, `1` for camera, and `2` for sphere. C++ constructs with the count only (`{ 1 }` for environment, `{ 1 }` for camera, and `( total )` for the sphere buffer in `polygon`) and binds explicitly with `bind( 0 )`, `bind( 1 )`, and `bind( 2 )`.
- GLSL field names equal the C++ member names with the `m_` prefix removed. GLSL struct names equal the C++ type names. The current accessors are `ssbo_<name>( uint )`, the block types are `<name>_buffer`, and the instances are `<name>_ssbo`.
- Payload layouts: `environment` is 7 floats (color, focal, ambient, volume). `camera` is 12 floats (position, forward, right, up, each 3 floats). `sphere` is 8 floats (position, radius, color).
- `sak::opengl` contains raw GL wrappers and the `shader` and `program` classes. It has no buffer or SSBO abstraction. That is not a reason to extend `sak`: the reflection work stays in the demo.
- The toolchain is g++ with C++26. The `cpp_compile` tool is configured for it.

## Decisions

- The fragment prelude stays authored. The `.glsl` fragment section keeps `#version 460 core`, the `resolution` uniform, `out vec4 final_color;`, helper macros such as `square`, and `main`.
- The generator emits only SSBO blocks, struct definitions, and accessors.
- The payload registry is an explicit ordered list: `environment`, `camera`, `sphere`.
- Binding is never part of construction. `shader_storage_buffer` allocates and uploads in its constructor and exposes a mandatory `bind( binding_index )` call afterwards. The class does not consult the registry; the index comes from the registry and is passed by the code responsible for aligning the current context with the program in use. A binding index is current-context state shared by every program, not program state, so the call belongs to that alignment step rather than to the buffer.
- All the code stays in `source/sdl3_demo.cpp`. The demo is both the experiment and the final location.
- The payload layout stays a flat float array with the existing `ssbo_header`. Typed `std430` arrays are rejected. In `std430`, `vec3` has a 16-byte base alignment. The C++ layout of `camera` places its four 12-byte vectors at offsets 0, 12, 24, and 36, but `std430` would place them at 0, 16, 32, and 48. `environment` is 28 bytes in C++, but a `std430` array of it would have a 32-byte stride. Matching `std430` would require padding the C++ payloads, which the no-padding rule forbids. Reads use float indices computed from `offset_of( member ).bytes / sizeof( float )`.

## Design

### Payload registry

- The registry is an ordered list of payload types. A type's binding is its position in the list, computed at compile time. The order `environment`, `camera`, `sphere` yields bindings 0, 1, and 2, which match today's values.
- `shader_storage_buffer` does not consult the registry. Its constructor takes only the element count; binding is a separate, mandatory `bind( binding_index )` call after construction.
- The registry is the source of the index that call receives. A payload type that is not in the registry fails a `static_assert` where the index is derived.

### Leaf type mapping

- `float` maps to `float`.
- The `sak::g3f` position and direction types map to `vec3`.
- The `sak::g3f` color type maps to `vec4`.
- A `static_assert` checks that each mapped type's `sizeof` equals its component count times `sizeof( float )`.
- An unmapped member type is a compile error.

### Generator

For each registry entry, a `template for` over `define_static_array( nonstatic_data_members_of( ^^t_payload, access_context::current( ) ) )` emits the following. The loop uses the same idiom the loader already uses for `shader::type`.

- `layout( std430, binding = N ) readonly buffer <name>_buffer { uint count; uint stride; float data[]; } <name>_ssbo;`, where `N` is the registry binding.
- `struct <name>` with one member per C++ member, in declaration order. Member names come from `identifier_of` with the `m_` prefix removed. Member types come from the leaf mapping.
- `<name> ssbo_<name>( uint index )`, which computes `base = index * <name>_ssbo.stride`, reads each member from `data[ base + k ]` where `k = offset_of( member ).bytes / sizeof( float )`, and builds vector members from consecutive reads.

The generator returns the whole block as a `string`, once per program load. The generated names must be identical to the current names, so `main` does not change.

A `static_assert` against the GLSL keyword and reserved-word list rejects member names that GLSL cannot use.

### Injection

- `shader_loader::load` inserts the generated block into each fragment section, immediately after the `#version` line. An `ensure` verifies that the section starts with `#version`.
- Immediately after the inserted block, the loader emits a `#line` directive. Its value is the line number, within that section, of the first authored line after `#version`, so driver errors point at authored lines.
- Vertex sections are not changed.
- The generated block must not reference any authored declaration.

### Authored fragment section after the change

The fragment section contains `#version 460 core`, the `resolution` uniform, `out vec4 final_color;`, the helper macros that stay authored, and `main`. The handwritten SSBO blocks, struct definitions, accessor functions, and the `vec3_from` and `vec4_from` macros are removed.

## Phases

Run one phase at a time. Each phase finishes and validates before the next starts. Sub-agents edit code and report what they changed. Only the main session builds. Build failures go back to the same sub-agent that made the change.

### Phase 1: registry and binding

1. Add the payload registry to `source/sdl3_demo.cpp`. The three payload structs stay in this file; no new header is created.
2. Replace the three hand-typed indices passed to `bind` in `main` with the index each payload type has in the registry. The constructor and the `bind` method already exist; `shader_storage_buffer` does not consult the registry, so the call site derives the index and passes it.
3. Do not change the GLSL.
4. Validate: the main session runs `cpp_compile`, then runs the demo. Behavior must match the pre-change build.

### Phase 2: generator

1. Implement the leaf mapping, the generator, and the static checks in `source/sdl3_demo.cpp`.
2. Create an ad-hoc test with `cpp_create_test` using the adhoc flag. The test prints the generated block. The main session builds it with `cpp_compile`.
3. Save the printed block and the current handwritten SSBO code from `data/shader/shadertoy.glsl` under `/tmp/opencode`. Compare names, member order, member types, and offsets with a tool such as `rg` and `diff` on extracted lines. Expression form may differ. Do not compare by reading.

### Phase 3: wire in

1. Before editing, the main session builds and runs the pre-change program and keeps its rendered output for comparison.
2. Implement the injection in `shader_loader`.
3. Delete the handwritten SSBO blocks, struct definitions, accessor functions, and the `vec3_from` and `vec4_from` macros from the fragment section.
4. The main session builds and runs the program. Compare the output with the pre-change output.

### Phase 4: validation

1. Run `cpp_code_verifier` on every changed C++ file.
2. Run the validator and fixer loop. One validator sub-agent checks the change against `all.md` and `cpp.md`. One fixer sub-agent applies the feedback. Repeat until the validator reports that the change satisfies the rules.
3. Run `cpp-include-tree --flg-auto-fix` after any include change.
4. The main session builds after every fixer pass that changed code.
5. After validation passes, run `git_quick_upload` with a message.

### Phase 5: documentation

1. Update this document with the final state.

## Constraints

- Only one buffer per payload type is supported. A second live buffer of the same type would rebind the first. Record this limit in the code.
- Only float-based payload members are supported. Other member types are compile errors until a bit-cast rule is added.
- Shader compile and link failures reach the demo's `main` through the `runtime_error` catch.
- Sub-agents must not build or compile.
- All C++ edits follow `docs/agent/style-guide/cpp.md`, including naming, the `m_` and `t_` prefixes, and spacing.
- The `cpp_compile` description in `../project-mcp-tools/README.md` says Clang. The user has stated the build is g++ with C++26. Do not change either as part of this task.
