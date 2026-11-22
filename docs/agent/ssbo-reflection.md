# SSBO layout

The CPU hands bulk data to the GPU as a flat array of floats. C++26 reflection validates that each payload is padding-free, and the GLSL side rebuilds the typed members of one element from consecutive floats. The buffer is raw memory on both sides, so the float order and the float offsets are the whole agreement between the two languages.

## The exchange

- A small header precedes the array: `{ uint count, uint stride }`. `count` is the number of elements and `stride` is the size of one element expressed in floats rather than bytes.
- The shader addresses element `index` with `base_index = index * stride` and reads each member at a fixed float offset from that base, so it needs no knowledge of byte sizes or alignment.
- The CPU owns the authoritative copy and the GPU receives copies at explicit points in time through `flush( )`. There is no mapping, no coherence protocol, and no shared memory.

## CPU payloads

| Payload | Members | Floats |
|---|---|---|
| `environment` | background color (4), focal (1), ambient (1), volume (1), half screen size (2), coordinate scale (1) | 10 |
| `camera` | position, forward, right, up, pixel right, pixel up (3 each) | 18 |
| `figure_payload` | position (3), type id (1), color (4), parameters (12) | 20 |

- The three payloads are declared in `source/figures_demo.cpp`. `figure_payload< t_parameter_count >` is instantiated as `figure` with a parameter capacity of twelve slots.
- A payload is a plain aggregate of trivially copyable members. The declaration is the only description of the layout; the float counts, the float offsets, and the GLSL members follow from it.

## is_flat

- The contract is declared in `include/sak/meta/meta.hpp`.
- `has_no_padding` reflects over `nonstatic_data_members_of` with `access_context::current( )`, walks the members in declaration order, and compares the accumulated `size_of( type_of( member ) )` against `offset_of( member ).bytes`. A leading gap, a gap between members, or a trailing gap fails the check, and the accumulated total must also equal `size_of( type )`.
- `is_flat` combines that check with `is_trivially_copyable_v`, so the payload has a contiguous byte image and no copy semantics that alter its bytes.
- `shader::storage_buffer` asserts `is_flat`, so a payload that gains padding fails at compile time on the declaration rather than at runtime in the shader.
- The concept constrains shape and copyability only; the element types of the members stay outside its reach.
- `m_type_id` travels as an exact integer-valued `float`, which represents every integer up to $2^{24}$ without loss, so the figure id survives the float transport unchanged.

## shader::storage_buffer

- The class is an RAII owner of one OpenGL buffer plus one heap array of payloads.
- The constructor takes only the element count: it allocates `unique_ptr< t_payload[ ] >` of that count, creates the buffer, and reserves `sizeof( ssbo_header ) + count * sizeof( t_payload )` bytes of dynamic storage.
- The header is written immediately, with `stride` computed as `sizeof( t_payload ) / sizeof( float )`. The float-unit stride is what makes the shader independent of byte sizes.
- The host-side array is reachable through `operator[ ]`, `data( )`, and `count( )`. Callers mutate elements in place, and those accesses upload nothing.
- `flush( )` uploads the payload region with one `glNamedBufferSubData` at offset `sizeof( ssbo_header )`, the exact size of that region.
- `bind( index )` performs `glBindBufferBase` for the shader storage target at the given binding index.
- Binding is separate from construction because a binding index is current-context state shared by every program, so allocation and binding are distinct steps and the buffer class never chooses an index on its own.

## GPU side

- Each payload has a block declaration that fixes the binding index, a struct mirroring the payload, and an accessor that rebuilds one element from the float array.

```glsl
layout( std430, binding = N ) readonly buffer <name>_buffer { uint count; uint stride; float data[]; } <name>_ssbo;
```

- The block type is `<name>_buffer` and the instance is `<name>_ssbo`, so the header members are read as `<name>_ssbo.count` and `<name>_ssbo.stride`. The header lets the shader loop over `count` and scale by `stride` without constants.
- `readonly` states that the shader only reads the buffer. The block uses `std430` as its layout qualifier, and the shader storage block itself is the construct that admits the runtime-sized `float data[ ]` array.
- The struct `<name>` restates each payload member in declaration order, with `float` for scalars and `vec2`, `vec3`, and `vec4` for the vector members.
- The accessor has the shape `ssbo_<name>( uint index )`. It computes `base_index = index * <name>_ssbo.stride`, reads scalars as `data[ base_index + k ]`, and assembles vectors from consecutive reads through the `vec2_from`, `vec3_from`, and `vec4_from` macros. The `k` values are the float offsets of the members.
- The current blocks are `environment_buffer`, `camera_buffer`, and `figure_buffer` at bindings `0`, `1`, and `2`. The single-element environment and camera buffers are read through `ssbo_environment( 0u )` and `ssbo_camera( 0u )`, and the figure buffer is traversed over `figure_ssbo.count`.

## Layout facts

| Payload | Floats | Bytes |
|---|---|---|
| `environment` | 10 | 40 |
| `camera` | 18 | 72 |
| `figure` | 20 | 80 |

- The byte count equals the float count times `sizeof( float )` because `is_flat` guarantees no gap between members.
- The GLSL member names are the C++ member names without the `m_` prefix, and the struct names equal the C++ type names, so the accessor names are those types prefixed with `ssbo_`.

## Why flat floats instead of typed std430 arrays

The alternative is to declare the block with the real struct type and let the driver apply `std430` to an array of it. That alternative mismatches the C++ layout, because `std430` imposes alignment that plain C++ aggregates do not.

- In `std430`, `vec3` has a 16-byte base alignment even though it occupies 12 bytes.
- `camera` is six consecutive `vec3`: 72 bytes in C++, with the vectors at byte offsets 0, 12, 24, 36, 48, and 60, whereas a `std430` array of the same struct uses a 96-byte stride.
- `environment` is 40 bytes in C++ and uses a 48-byte stride under `std430`, because its `vec4` background color gives the struct a 16-byte alignment.
- `figure` mixes a `vec3`, a `float`, a `vec4`, and a float array, so a typed block would apply the same alignment rules member by member.
- Padding members in the C++ payloads would align the two layouts, and `is_flat` rejects padding, so the padded form stays outside the payload set.
- The float array keeps only the order of the floats and their float offsets as the agreement, since alignment and padding are properties of the block members rather than of the `float` elements.
- The cost is that vector members lose their typed form in the buffer and are rebuilt on the GPU from consecutive floats, which is what the `vec*_from` macros do.

## Current demo

- `source/figures_demo.cpp` renders with `data/shader/figures.glsl` and the figure blocks under `data/shader/figures/`.
- The demo declares the three payloads, binds them at `0`, `1`, and `2`, fills one environment element, one camera element, and one figure element per registered figure, and flushes each buffer after mutation.
- The fragment template carries the block declarations, the mirrors, and the accessors; the demo loads it through `text_file`, splits it with `shader::split_sections`, and compiles the section map with `shader::compile`.

## Architectural limits

- One live buffer per payload type, because a binding index is context state and a second buffer of the same type bound to the same index replaces the first.
- Float-based payload members, because the block exposes a `float data[ ]` array and the accessors rebuild only members that are `float` or aggregates of `float`.
- A flat float representation, because a typed `std430` block reintroduces the alignment mismatch described above.
