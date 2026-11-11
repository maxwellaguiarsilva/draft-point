# Baseline Reference

## Scope

- Reference architecture of the immutable sphere renderer in `source/sdl3_shadertoy.cpp` and `data/shader/sdl3-shadertoy.glsl`:
  - Full-screen vertexless raster pass and screen-to-ray pinhole projection.
  - Flat-float SSBO header layout and manual GLSL struct reconstruction.
  - Analytical closest-approach sphere inclination shading and camera movement loop.
  - Baseline assumptions that hold specifically for disjoint equal-radius spheres and are generalized in [Agnostic Pipeline](agnostic-pipeline.md).

## Verified Architecture

### Host Full-Screen Pass

- Geometry submission (`source/sdl3_shadertoy.cpp` `demo_logic::run`, `data/shader/sdl3-shadertoy.glsl`):
  - Host binds an empty `sak::opengl::vertex_array` and calls `gl_draw_arrays( GL_TRIANGLE_STRIP, 0, 4 )` once per frame.
  - Vertex shader indexes a 4-element constant clip-space quad directly from `gl_VertexID`.
  - Fragment shader computes view rays directly from `gl_FragCoord.xy` and SSBO payloads.

### Flat-Float SSBO Header and Unpacking

- Buffer management (`include/sak/opengl/shader.hpp`, [SSBO Reflection](../../ssbo-reflection.md)):
  - `sak::opengl::shader::storage_buffer< t_payload >` enforces `is_flat< t_payload >` (`has_no_padding( ^^t_payload )` and `is_trivially_copyable_v< t_payload >`).
  - Construction allocates `sizeof( ssbo_header ) + count * sizeof( t_payload )` bytes and uploads `ssbo_header{ count, sizeof( t_payload ) / sizeof( float ) }` at byte offset `0`.
  - `flush( )` uploads the contiguous host array at byte offset `sizeof( ssbo_header )`, and `bind( index )` attaches the buffer to its GLSL binding point.
- Device unpacking (`data/shader/sdl3-shadertoy.glsl`):
  - Each block declares `layout( std430, binding = slot_index ) readonly buffer <name>_buffer { uint count; uint stride; float data[ ]; } <name>_ssbo;`.
  - Each `ssbo_<name>( uint index )` accessor computes `uint base_index = index * <name>_ssbo.stride;` and rebuilds vector fields through:
    - `vec2_from`
    - `vec3_from`
    - `vec4_from`

### Payload Layouts

| Binding | C++ / GLSL Type | Ordered Members (Float Count) | Floats | Bytes | GLSL Accessor |
|---|---|---|---|---|---|
| `0` | `environment` | `m_background_color` (4), `m_focal` (1), `m_ambient` (1), `m_volume` (1), `m_half_screen_size` (2), `m_coordinate_scale` (1) | 10 | 40 | `ssbo_environment( 0u )` |
| `1` | `camera` | `m_position` (3), `m_forward` (3), `m_right` (3), `m_up` (3) | 12 | 48 | `ssbo_camera( 0u )` |
| `2` | `sphere` | `m_position` (3), `m_radius` (1), `m_color` (4) | 8 | 32 | `ssbo_sphere( index )` |

- Evidence:
  - Host structs in `source/sdl3_shadertoy.cpp`:
    - `gl::environment`
    - `gl::camera`
    - `gl::sphere`
  - Matching GLSL structs and `ssbo_*` accessors in `data/shader/sdl3-shadertoy.glsl`.

### Scene and Camera Baseline Configuration

| Buffer Element | Initial Values | Evidence |
|---|---|---|
| `m_environment[ 0 ]` | `background_color = { 0.0f, 0.0f, 0.0f, 1.0f }`, `focal = 2.0f`, `ambient = 0.3f`, `volume = 0.7f` | `source/sdl3_shadertoy.cpp` (`demo_logic::demo_logic`) |
| `m_camera[ 0 ]` | `position = { 0.0f, 0.0f, -2.0f }`, `forward = { 0.0f, 0.0f, 1.0f }`, `right = { 1.0f, 0.0f, 0.0f }`, `up = { 0.0f, 1.0f, 0.0f }` | `source/sdl3_shadertoy.cpp` (`demo_logic::demo_logic`) |
| `m_spheres[ 0 ]` | `position = { -0.55f, 0.0f, 1.0f }`, `radius = 0.45f`, `color = { 1.0f, 0.0f, 0.0f, 1.0f }` (red, left) | `source/sdl3_shadertoy.cpp`, mirrored in `tests/adhoc/0010_sdl3_demo.cpp` |
| `m_spheres[ 1 ]` | `position = { 0.55f, 0.0f, 1.0f }`, `radius = 0.45f`, `color = { 0.0f, 0.0f, 1.0f, 1.0f }` (blue, right) | `source/sdl3_shadertoy.cpp`, mirrored in `tests/adhoc/0010_sdl3_demo.cpp` |

### Screen-to-Ray Projection

- Pixel centering and aspect normalization (`data/shader/sdl3-shadertoy.glsl` `main`):
  - `vec2 input_coord = ( gl_FragCoord.xy - s_environment.half_screen_size ) * s_environment.coordinate_scale;`
  - Maps the shorter window dimension onto `[ -1.0, +1.0 ]` while preserving square pixels across arbitrary aspect ratios.
- Pinhole ray construction:
  - Origin: `s_camera.position`
  - Unnormalized world direction: `vec3 direction = s_camera.forward * s_environment.focal + s_camera.right * input_coord.x + s_camera.up * input_coord.y;`

### Sphere Closest-Approach Inclination Formula

| Quantity | Baseline Expression (`data/shader/sdl3-shadertoy.glsl`) | Geometric Meaning |
|---|---|---|
| `hypotenuse` | `s_sphere.position - s_camera.position` | Vector from camera origin to sphere center |
| `dot_hypotenuse_direction` | `dot( hypotenuse, direction )` | Scaled forward projection; hit requires `> 0.0` |
| `opposite_leg_squared` | `dot( hypotenuse, hypotenuse ) - square( dot_hypotenuse_direction ) / dot( direction, direction )` | Squared perpendicular distance $d^2$ from sphere center to ray line |
| `ratio` | `opposite_leg_squared / square( s_sphere.radius )` | Normalized squared radial miss $(d / R)^2$; hit requires `<= 1.0` |
| `cos_incidence` | `sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) )` | Exact $|\hat{\mathbf{r}} \cdot \hat{\mathbf{n}}|$ at the sphere entry point (`1.0` at silhouette center, `0.0` at grazing silhouette edge) |
| `color` | `s_sphere.color.rgb * ( s_environment.ambient + s_environment.volume * cos_incidence )` | Final shaded RGB computed directly from analytical ray-sphere inclination and base color |

### Resize and Input Loop

- Window resize (`demo_logic::pixel_resize` in `source/sdl3_shadertoy.cpp`):
  - Converts window pixel dimensions to `planar_point`.
  - Stores `m_half_screen_size = screen_size / 2.0f` and `m_coordinate_scale = 2.0f / min( screen_size )` using the `sak::math::min` range reduction, then calls `m_environment.flush( )`.
- Camera controls in `source/sdl3_shadertoy.cpp`:
  - `demo_logic` input handlers:
    - `key_movements`
    - `key_down`
    - `key_up`
    - `focus_lost`
    - `update`
  - Planar movement bindings:
    - `SDLK_W` and `SDLK_S` drive movement axis `1` (`m_forward`).
    - `SDLK_A` and `SDLK_D` drive movement axis `0` (`m_right`).
    - Movement axis `2` (`m_up`) stays `0.0f`.
  - Window and focus actions:
    - `SDLK_F11` toggles fullscreen.
    - `SDLK_ESCAPE` quits the application.
    - `focus_lost` resets `m_movement` to zero.
  - `update` integrates `current.m_position += world_direction * ( camera_speed * delta_seconds )` at `camera_speed = 1.5f` and flushes `m_camera`.

## Baseline Simplifications Superseded Downstream

| Baseline Simplification | Behavior in `sdl3-shadertoy.glsl` | Why It Holds in Baseline | Upgraded Contract Downstream |
|---|---|---|---|
| Unnormalized Ray Direction | `direction` retains length $\sqrt{\text{focal}^2 + x^2 + y^2}$ and each sphere divides by `dot( direction, direction )` inside the loop | Single primitive formula compensates for non-unit ray length via division | [Agnostic Pipeline](agnostic-pipeline.md) normalizes `vec3 unit_direction = normalize( direction );` once per fragment before the figure loop, expressing ray distances directly in world-space metric units |
| Center Distance vs Surface `hit_distance` | Depth test compares `distance = length( hypotenuse )` (camera-to-center distance) | Two disjoint exterior spheres of equal radius (`0.45f`) have identical center-order and surface-entry order | [Agnostic Pipeline](agnostic-pipeline.md) orders hits by world-metric surface entry `hit_distance > 0.0` (`along - radius * cos_incidence` for exterior spheres), supporting general primitive surfaces in [Rounded Primitives](rounded-primitives.md) and [Adaptive Mesh](adaptive-mesh.md) |
| Hardcoded Single-Figure Loop | `sphere_ssbo` and the sphere formula are hardcoded inside `main( )` | Baseline scene contains only spheres | [Agnostic Pipeline](agnostic-pipeline.md) binds a fixed-stride heterogeneous `figure_ssbo` and injects per-figure GLSL evaluators and type dispatch from the CPU |

## Boundary

- This baseline owns the immutable reference executable (`source/sdl3_shadertoy.cpp`) and shader (`data/shader/sdl3-shadertoy.glsl`).
- [Agnostic Pipeline](agnostic-pipeline.md) owns:
  - Heterogeneous flat figure buffers.
  - Per-fragment normalized `unit_direction` dispatch.
  - World-metric surface `hit_distance` depth sorting.
- Object and camera rotational algebra belong to [Orientation](orientation.md).
- Sub-linear visibility traversal belongs to [Spatial Graph](spatial-graph.md).

## Relations

- Parent domain: [3D Renderer](index.md)
- Buffer layout specification: [SSBO Reflection](../../ssbo-reflection.md)
- Direct consumer: [Agnostic Pipeline](agnostic-pipeline.md)
