# Baseline Reference

Import: [3D Renderer](index.md)

## Scope

- Contract of the existing sphere renderer that every later stage reproduces or replaces: `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl`.
- Records the data layout handed to the GPU, the projection, and the inclination formula that produces the perfectly round 3D look.

## Facts

### Host and Pixel Feed

- The host draws a full-screen quad with `gl_draw_arrays( GL_TRIANGLE_STRIP, 0, 4 )`; the vertex shader emits the four positions from `gl_VertexID`, so no vertex buffer or attribute exists.
- Screen projection is computed in the fragment shader from `gl_FragCoord` against an environment SSBO field, so the CPU sends no matrices at all.
- Evidence: `source/sdl3_shadertoy.cpp` `demo_logic::run` and `pixel_resize`; `data/shader/sdl3-shadertoy.glsl` vertex section.

### Storage Buffers

- Three storage buffers at bindings `0`, `1` and `2`: environment, camera, sphere list.
- Every buffer is prefixed with `uint count; uint stride; float data[];` under `std430`; the shader assembles its structs from flat floats using the `vec2_from`/`vec3_from`/`vec4_from` macros.
- The header and stride are written by `sak::opengl::shader::storage_buffer`, not by the caller.
- Evidence: `glsl` buffer declarations and `ssbo_*` accessors; `include/sak/opengl/shader.hpp` `storage_buffer::storage_buffer`.

### Payload Layouts

- `environment`: background color (4) + `focal` + `ambient` + `volume` + half screen size (2) + `coordinate_scale` = ten floats.
- `camera`: `position`, `forward`, `right`, `up` as three floats each = twelve floats.
- `sphere`: `position` (3) + `radius` + `color` (4) = eight floats.
- Evidence: the three structs declared in `source/sdl3_shadertoy.cpp` with their float-count comments, mirrored by the accessors in the `glsl` file.

### Scene and Camera Values

- Two spheres at `x = -0.55` (red) and `x = +0.55` (blue), both at `y = 0`, `z = 1`, radius `0.45`.
- Camera position `{ 0, 0, -2 }`, forward `{ 0, 0, 1 }`, right `{ 1, 0, 0 }`, up `{ 0, 1, 0 }`.
- Environment: `focal = 2.0`, `ambient = 0.3`, `volume = 0.7`, black background.
- Evidence: `demo_logic` constructor; the same values are repeated in `tests/adhoc/0010_sdl3_demo.cpp`, so the two prototypes are visually interchangeable at startup.

### Inclination Formula

- `hypotenuse = sphere_position - camera_position`
- `along = dot( hypotenuse, direction )`; a hit requires `along > 0`
- `opposite_leg_squared = dot( hypotenuse, hypotenuse ) - square( along ) / dot( direction, direction )`
- `ratio = opposite_leg_squared / square( radius )`; a hit requires `ratio <= 1`
- `brightness = ambient + volume * sqrt( 1 - clamp( ratio, 0, 1 ) )`
- Evidence: the fragment `main` loop of the `glsl` file.

- The brightness depends only on `ratio`, the normalized squared distance from the ray to the sphere center. That single scalar is what reads as a perfect round surface: no normal vector, no light direction, no tessellation.

### Resize Handling

- `pixel_resize` stores half the pixel size and `coordinate_scale = 2 / min( screen size )`, then flushes the environment buffer. `min` is the `sak::math::min` binary niebloid.
- Evidence: `demo_logic::pixel_resize`.

### Camera Movement

- `W`/`S` map to axis `1` (forward), `A`/`D` map to axis `0` (right); axis `2` (up) is declared in the movement table but has no key bound to it.
- The world direction is composed as `right * x + forward * y + up * z` and applied to the position at a fixed speed, then the camera buffer is flushed.
- Evidence: `key_movements`, `key_down`, `key_up`, `update`.

## Exclusions

- Depth ordering uses `length( hypotenuse )`, the distance to the sphere center, not the entry-point distance along the ray. Two spheres of the same radius are therefore ordered by center distance; the current scene never exposes the difference.
- The GPU knows exactly one figure type. The sphere loop is hardcoded over a single `sphere` struct, and the SSBO type is fixed at compile time by `storage_buffer< sphere >`.
- There is no normal, no light vector, no shadowing, no texture, and no culling. Anything observing otherwise in a later version is a change introduced after this baseline.

## Relations

- [Agnostic Pipeline](agnostic-pipeline.md) replaces the hardcoded `sphere` loop and the fixed payload while preserving the inclination brightness of this document.
- [Orientation](orientation.md) adds the first orientation data this baseline does not have.
