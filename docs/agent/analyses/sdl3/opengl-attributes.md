# OpenGL Attributes (SDL3)

- Source of truth: SDL3 header `SDL_video.h` (`\since SDL 3.2.0`).
- Scope: `SDL_GLAttr` enumeration attributes set through `SDL_GL_SetAttribute(SDL_GLAttr attr, int value)`.
- Context creation fails only if the GL cannot provide a minimum request; the driver may return more.
- Prefix `SDL_GL_` is declared here and omitted from attribute names below; supporting type names keep their prefix.

## Value domains

### Framebuffer bit counts

- `RED_SIZE`
- `GREEN_SIZE`
- `BLUE_SIZE`
- `ALPHA_SIZE`
- `BUFFER_SIZE`
- `DEPTH_SIZE`
- `STENCIL_SIZE`
- `ACCUM_RED_SIZE`
- `ACCUM_GREEN_SIZE`
- `ACCUM_BLUE_SIZE`
- `ACCUM_ALPHA_SIZE`
- `FLOATBUFFERS`

### Boolean & visual requests

- `DOUBLEBUFFER`
- `STEREO`
- `ACCELERATED_VISUAL` — select hardware acceleration or software rendering; allows either if unspecified.
- `SHARE_WITH_CURRENT_CONTEXT`
- `FRAMEBUFFER_SRGB_CAPABLE` — request only.
- `CONTEXT_NO_ERROR`
- `RETAINED_BACKING` — deprecated; do not use.

### Multisampling

- `MULTISAMPLEBUFFERS`
- `MULTISAMPLESAMPLES`

### Context version & policy

- `CONTEXT_MAJOR_VERSION`
- `CONTEXT_MINOR_VERSION`
- `CONTEXT_FLAGS` — mask of `SDL_GLContextFlag`; members share prefix `SDL_GL_CONTEXT_`:
  - `DEBUG_FLAG`
  - `FORWARD_COMPATIBLE_FLAG`
  - `ROBUST_ACCESS_FLAG`
  - `RESET_ISOLATION_FLAG`
- `CONTEXT_PROFILE_MASK` — profile type per `SDL_GLProfile`; members share prefix `SDL_GL_CONTEXT_PROFILE_`:
  - `CORE`
  - `COMPATIBILITY`
  - `ES`
- `CONTEXT_RELEASE_BEHAVIOR` — release behavior per `SDL_GLContextReleaseFlag`; members share prefix `SDL_GL_CONTEXT_RELEASE_BEHAVIOR_`:
  - `NONE`
  - `FLUSH`
- `CONTEXT_RESET_NOTIFICATION` — reset notification per `SDL_GLContextResetNotification`; members share prefix `SDL_GL_CONTEXT_RESET_`:
  - `NO_NOTIFICATION`
  - `LOSE_CONTEXT`

### Platform

- `EGL_PLATFORM` — string or pointer semantics instead of integer value domain.
