# SSBO Reflection

The file name joins two ideas. *SSBO* is the shader storage buffer object, the OpenGL buffer type through which the CPU hands bulk data to a shader. *Reflection* is the C++26 compile-time inspection of a type: its members, their order, their types, their sizes, and their byte offsets. The document describes a technique for deriving the GLSL side of the exchange from the reflection of the C++ side, so that a single declaration defines the contract instead of two handwritten copies of it.

The reflection generator does not exist in the code yet. The programs described here still carry the handwritten GLSL form, and the reflection direction is the design this document records as the goal of the technique.

## Why the document exists

- A shader storage buffer is raw memory on both sides. Nothing in OpenGL validates that the C++ struct and the GLSL block describe the same layout.
- CPU and GPU must agree on four things at once: the byte layout of each payload, the order of the fields, the names of the fields, and the binding index of the buffer.
- The handwritten form duplicates the C++ declaration inside the `.glsl` file. Every field is restated as a GLSL member, every offset is restated as a numeric literal, and every binding is restated as an integer on both sides.
- A duplicated declaration can drift. Adding, removing, reordering, or retyping a C++ member leaves the GLSL copy valid and compilable, so the divergence appears only as wrong pixels or as silent misreads of unrelated fields.
- Reflection removes the duplication: the GLSL declarations become a function of the C++ types, and a change to a payload propagates to the shader by construction.

## The exchange

- The payload travels as a plain array of floats. Neither side ever shares a struct, a pointer, or a typed block layout.
- A small header precedes the array: `uint count` and `uint stride`. `count` is the number of elements in the buffer, and `stride` is the size of one element expressed in floats rather than bytes.
- The stride lets the shader address element `index` with `base_index = index * stride` and then read fields at fixed float offsets from that base, so the shader needs no knowledge of byte sizes or alignment.
- The CPU side owns the authoritative copy and the GPU side receives a copy at explicit points in time. There is no mapping, no coherence protocol, and no shared memory.

## CPU side

### Payloads

- A payload is a plain aggregate of trivially copyable members, declared in the file that owns the data.
- `environment` carries the frame-independent scene state: background color, focal length, ambient term, volume term, half screen size, and the coordinate scale derived from the pixel size.
- `camera` carries the view basis: position, forward, right, and up.
- `sphere` carries one scene element: position, radius, and color.
- The payload types are the only description of the layout. The float counts, the offsets, and the GLSL members are all consequences of these declarations.

### `is_flat`

- `has_no_padding` reflects over `nonstatic_data_members_of` with `access_context::current( )` and walks the members in declaration order, accumulating `size_of( type_of( member ) )`.
- For each member it compares the accumulated offset against `offset_of( member ).bytes`. Any gap between members, any leading gap, or any trailing gap makes the check fail: the accumulated total must also equal `size_of( type )`.
- `is_flat` combines that check with `is_trivially_copyable_v`, so the payload has a contiguous byte image and no copy semantics that would alter its bytes.
- `shader::storage_buffer` asserts the concept, which means a payload that gains padding cannot be uploaded, and the failure happens at compile time on the declaration instead of at runtime in the shader.
- The concept does not inspect the element types of the members. It constrains shape and copyability only.

### `shader::storage_buffer`

- The class is an RAII owner of one OpenGL buffer plus one heap array of payloads.
- The constructor takes only the element count. It allocates `unique_ptr<t_payload[]>` of that count, creates the buffer, and reserves `sizeof( ssbo_header ) + count * sizeof( t_payload )` bytes of dynamic storage.
- It writes the header immediately, computing `stride` as `sizeof( t_payload ) / sizeof( float )`. The float-unit stride is what makes the shader independent of byte sizes.
- The host-side array is reachable through `operator[]`, `data( )`, and `count( )`. Callers mutate elements in place; nothing is uploaded by those accesses.
- `flush( )` uploads the whole array with one `glNamedBufferSubData` at offset `sizeof( ssbo_header )`, the exact size of the payload region.
- `bind( index )` performs `glBindBufferBase` for the shader storage target at the given binding index.
- Binding is separate from construction. A binding index is current-context state shared by every program, not a property of the buffer, so allocation and binding are distinct steps and the buffer class never decides an index on its own.
- A destructor object owned through `unique_ptr` deletes the buffer, and the payload array is released with the class, so both resources follow the same lifetime.

## GPU side

- Each payload has a block declaration that fixes the binding index, then a struct that mirrors the payload, then an accessor that rebuilds one element from the float array.

```glsl
layout( std430, binding = N ) readonly buffer <name>_buffer { uint count; uint stride; float data[]; } <name>_ssbo;
```

- The block carries the same `count` and `stride` header the CPU wrote, so the shader can loop over `count` and scale by `stride` without constants.
- The block type is `<name>_buffer` and the instance is `<name>_ssbo`, so the header members are read as `<name>_ssbo.count` and `<name>_ssbo.stride`.
- `readonly` states that the shader only reads the buffer. The block uses `std430` as its layout qualifier, and the shader storage block itself is the construct that admits the runtime-sized `float data[]` array.
- The struct `<name>` restates each payload member in declaration order, with GLSL types chosen by kind: `float` for scalars, `vec2`, `vec3`, and `vec4` for the vector members.
- The accessor has the shape `ssbo_<name>( uint index )`. It computes `base_index = index * <name>_ssbo.stride`, reads scalars as `data[ base_index + k ]`, and assembles vectors from consecutive reads through the `vec2_from`, `vec3_from`, and `vec4_from` macros.
- The `k` values are the float offsets of the members. They are literals in the handwritten form: `0`, `3`, `4`, and so on, each derived by hand from the C++ declaration.
- `main` calls `ssbo_environment( 0u )` and `ssbo_camera( 0u )` once, and `ssbo_sphere( index )` inside a loop bounded by `sphere_ssbo.count`.
- The pixel size is not a uniform. Half of it reaches the fragment shader as the `half_screen_size` member of the `environment` payload and is refreshed whenever the window size changes, so centering a fragment is a subtraction of that member rather than a multiplication by `0.5`. The derived `2 / min( width, height )` factor is still computed on the CPU at that same moment and travels as the `coordinate_scale` member, so the fragment shader multiplies by a constant instead of dividing per fragment.

## Layout facts

| payload | members | floats | bytes |
| --- | --- | --- | --- |
| `environment` | color (4), focal (1), ambient (1), volume (1), half screen size (2), coordinate scale (1) | 10 | 40 |
| `camera` | position (3), forward (3), right (3), up (3) | 12 | 48 |
| `sphere` | position (3), radius (1), color (4) | 8 | 32 |

- The byte counts equal the float counts because every member is a `float` or an aggregate of `float`, and `is_flat` guarantees no gap between them.
- The GLSL field names are the C++ member names without the `m_` prefix, so `m_background_color` appears as `background_color`, `m_half_screen_size` as `half_screen_size`, and `m_coordinate_scale` as `coordinate_scale`.
- The GLSL struct names equal the C++ type names, and the accessor names are those types prefixed with `ssbo_`.
- The binding indices are `0` for `environment`, `1` for `camera`, and `2` for `sphere`, and they appear once in the GLSL block declaration and once in the CPU `bind` call.

## Why flat floats instead of typed `std430` arrays

The obvious alternative is to declare the block with the real struct type and let the driver apply `std430` to an array of it. That alternative does not match the C++ layout, because `std430` imposes alignment that plain C++ aggregates do not.

- In `std430`, `vec3` has a 16-byte base alignment even though it occupies 12 bytes.
- `camera` is four consecutive `vec3`. C++ places them at byte offsets 0, 12, 24, and 36, for a total of 48 bytes. A `std430` array of the same struct would place them at 0, 16, 32, and 48, for a total of 64 bytes, with four bytes of padding after each vector.
- `environment` begins with a `vec4` background color, which gives the struct a 16-byte alignment under `std430`. The C++ type is 40 bytes, but a `std430` array of it would use a 48-byte stride, so every element after the first would be read from the wrong place.
- Matching `std430` would require adding padding members to the C++ payloads so both sides agree. The payloads are deliberately padding-free, and `is_flat` rejects any padding, so the padded form is not available.
- The float array avoids the whole question. Alignment and padding are properties of the block members, not of the `float` elements, so the only remaining agreement is the order of the floats and their offsets, which reflection can derive.
- The generator derives reads from `offset_of( member ).bytes / sizeof( float )`, converting a C++ byte offset into a float index. Because `is_flat` proves the layout is contiguous, that division is exact and no rounding or alignment correction is involved. The handwritten accessors state the same offsets as integer literals.
- The cost is that vector members lose their typed form in the buffer and must be rebuilt on the GPU from consecutive floats, which is what the `vec*_from` macros do.

## Reflection direction

The direction recorded by this document replaces the handwritten declarations with generated ones.

### Payload registry

- The registry is an ordered list of payload types. A type's binding index is its position in the list, computed at compile time, so `environment`, `camera`, and `sphere` yield 0, 1, and 2.
- The buffer class does not consult the registry. It still allocates from a count, and the index it is bound with comes from the registry at the call site.
- A payload type absent from the registry fails a `static_assert` where the index is derived, so a payload cannot be uploaded without a binding.

### Leaf type mapping

- A small table maps each supported member type to a GLSL type: `float` to `float`, the 3-component position and direction types to `vec3`, the 4-component color type to `vec4`, and the 2-component planar point to `vec2`.
- A `static_assert` checks that each mapped type's `sizeof` equals its component count times `sizeof( float )`, so the mapping itself is verified against the size the float arithmetic assumes.
- An unmapped member type is a compile error, which keeps the supported set closed and explicit.

### Generator

- For each registry entry, a `template for` iterates `define_static_array( nonstatic_data_members_of( ^^t_payload, access_context::current( ) ) )`. This is the same idiom the shader loader already uses to walk `enumerators_of( ^^type )`.
- The loop emits the block declaration with the registry binding, then the struct with one member per payload member in declaration order.
- Member names come from `identifier_of` with the `m_` prefix removed, and member types come from the leaf mapping. The result reproduces the current handwritten names exactly, so `main` keeps compiling against them.
- The loop also emits the accessor `ssbo_<name>( uint index )`, computing `base = index * <name>_ssbo.stride`, reading `data[ base + k ]` with `k = offset_of( member ).bytes / sizeof( float )`, and assembling vector members from consecutive reads.
- The generated text is a function of the payload types alone. It references no authored declaration, so it can be inserted anywhere the fragment shader has the required types available.
- A `static_assert` against the GLSL keyword and reserved-word list rejects member names that GLSL cannot use, since a valid C++ identifier is not necessarily a valid GLSL one.

### Injection

- The generated block belongs in the fragment section, immediately after the `#version` line, because the block declaration must precede the accessors and any use of them.
- A `#line` directive follows the inserted block and restores the authored numbering, so driver error messages report line numbers of the authored text rather than of the expanded source.
- The insertion point is the `shader::loader` class, since it already splits the combined file into sections and constructs one shader per section. The generated text is produced once per program load.
- Vertex sections are unaffected. They declare no storage blocks and use no vertex attributes: the corner positions come from a constant array indexed by the built-in `gl_VertexID`.

## Where the technique is demonstrated

- `tests/adhoc/0010_sdl3_demo.cpp` with `data/shader/sdl3-demo.glsl` is the primary case. The CPU side builds a variable number of spheres, and the polygon geometry is uploaded through `shader::storage_buffer<sphere>`; the fragment shader loops over `sphere_ssbo.count` and picks the nearest hit.
- `source/sdl3_shadertoy.cpp` with `data/shader/sdl3-shadertoy.glsl` is the static variant. It holds two fixed spheres and moves the camera, so it exercises the same payloads and the same GLSL blocks with a fixed count.
- The payload declarations and the handwritten GLSL blocks remain duplicated in both files, while `has_no_padding`, `is_flat`, and `shader::storage_buffer` come from `sak/opengl/shader.hpp`.

## Architectural limits

- One live buffer per payload type. A binding index is context state, so a second buffer of the same type bound to the same index replaces the first, and both the CPU and the shader would then address the wrong storage.
- Only float-based payload members. The block exposes a `float data[]` array, so the current handwritten accessors can reconstruct only members that are `float` or aggregates of `float`. The reflection direction makes the same limit explicit: the leaf mapping turns an unsupported member type into a compile error.
- The representation in use is a flat float array. Declaring the block with the payload struct type would reintroduce the alignment mismatch described above, so the flat float array is what both programs upload and read.
