# OpenGL Attributes (SDL3)

## Source of truth

List extracted from the local SDL3 header `SDL_video.h`. The `SDL_GLAttr` enumeration is available since SDL 3.2.0. Re-verify against the installed header if the SDL version changes.

All attributes are set through `SDL_GL_SetAttribute(SDL_GLAttr attr, int value)`, which takes an `int` value. Context creation only fails if the GL cannot provide a *minimum* request; the driver may return more (e.g. asking for 16 depth bits may yield 24).

## Attributes grouped by value domain

The separation below is by the *nature of the value* each attribute carries, which is what allows each group to become its own strongly-typed enum for a `set` method.

### Visual / framebuffer (bit counts)

- `SDL_GL_RED_SIZE` — minimum bits for the red channel of the color buffer; defaults to 8.
- `SDL_GL_GREEN_SIZE` — minimum bits for the green channel of the color buffer; defaults to 8.
- `SDL_GL_BLUE_SIZE` — minimum bits for the blue channel of the color buffer; defaults to 8.
- `SDL_GL_ALPHA_SIZE` — minimum bits for the alpha channel of the color buffer; defaults to 8.
- `SDL_GL_BUFFER_SIZE` — minimum bits for the frame buffer size; defaults to 0.
- `SDL_GL_DEPTH_SIZE` — minimum bits in the depth buffer; defaults to 16.
- `SDL_GL_STENCIL_SIZE` — minimum bits in the stencil buffer; defaults to 0.
- `SDL_GL_ACCUM_RED_SIZE` / `SDL_GL_ACCUM_GREEN_SIZE` / `SDL_GL_ACCUM_BLUE_SIZE` / `SDL_GL_ACCUM_ALPHA_SIZE` — minimum bits for each channel of the accumulation buffer; defaults to 0.
- `SDL_GL_FLOATBUFFERS` — floating-point framebuffers (0/1).

### Boolean (0/1, or -1 for "don't care")

- `SDL_GL_DOUBLEBUFFER` — single or double buffered output; defaults to double buffering on.
- `SDL_GL_STEREO` — stereo 3D output; defaults to off.
- `SDL_GL_ACCELERATED_VISUAL` — 1 requires hardware acceleration, 0 forces software rendering; defaults to allow either.
- `SDL_GL_SHARE_WITH_CURRENT_CONTEXT` — share the current context; defaults to 0.
- `SDL_GL_FRAMEBUFFER_SRGB_CAPABLE` — requests an sRGB-capable visual when 1; defaults to -1 ("don't care"); a request only.
- `SDL_GL_CONTEXT_NO_ERROR` — context without error reporting (0/1).
- `SDL_GL_RETAINED_BACKING` — **deprecated; do not use.**

### Multisampling (counts)

- `SDL_GL_MULTISAMPLEBUFFERS` — number of buffers used for multisample anti-aliasing; defaults to 0.
- `SDL_GL_MULTISAMPLESAMPLES` — number of samples around the current pixel used for multisample anti-aliasing.

### Version / context type (own domains)

- `SDL_GL_CONTEXT_MAJOR_VERSION` / `SDL_GL_CONTEXT_MINOR_VERSION` — OpenGL context version.
- `SDL_GL_CONTEXT_FLAGS` — combination of `SDL_GLContextFlag` values: `SDL_GL_CONTEXT_DEBUG_FLAG` (0x1), `SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG` (0x2), `SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG` (0x4), `SDL_GL_CONTEXT_RESET_ISOLATION_FLAG` (0x8); defaults to 0.
- `SDL_GL_CONTEXT_PROFILE_MASK` — context type per `SDL_GLProfile`: `SDL_GL_CONTEXT_PROFILE_CORE` (0x1), `SDL_GL_CONTEXT_PROFILE_COMPATIBILITY` (0x2), `SDL_GL_CONTEXT_PROFILE_ES` (0x4); default depends on platform.
- `SDL_GL_CONTEXT_RELEASE_BEHAVIOR` — release behavior per `SDL_GLContextReleaseFlag`: `SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE` (0x0), `SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH` (0x1); defaults to FLUSH.
- `SDL_GL_CONTEXT_RESET_NOTIFICATION` — reset notification per `SDL_GLContextResetNotification`: `SDL_GL_CONTEXT_RESET_NO_NOTIFICATION` (0x0), `SDL_GL_CONTEXT_RESET_LOSE_CONTEXT` (0x1); defaults to NO_NOTIFICATION.

### Other

- `SDL_GL_EGL_PLATFORM` — the EGL platform; the only attribute whose value is not a simple `int` bit count/flag (string/pointer semantics).

## Note on the "data type" split

`SDL_GL_EGL_PLATFORM` is the only genuinely atypical attribute. The rest are all integers, so in practice the useful separation for strong typing is by *value domain* (flag bitmask, profile, release behavior, reset notification, boolean, bit count), giving each domain its own typed enum rather than by raw C integer type.
