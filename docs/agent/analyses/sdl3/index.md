# SDL3

## Objective

- Gather information that will help build a RAII, object-oriented C++ library for SDL3, following the resources and patterns of the `sak` library.
- The resulting API lives under the `sak/sdl3` hierarchy (`include/sak/sdl3/`).
- Each SDL3 object is analyzed in its own subject markdown; this index points to them.

## Strategy

The strategy for analyzing and handling this problem is defined 100% by the owner, minutely, through specific instructions passed along the way.

## Nodes

- [Application](application.md) [#current] — RAII wrapper for the SDL3 application lifecycle.
- [Display](display.md) — RAII wrapper for an SDL3 display.
- [Window](window.md) — RAII wrapper for an SDL3 window.
- [OpenGL Context](opengl-context.md) — RAII wrapper for an SDL3 OpenGL context.
- [OpenGL Attributes](opengl-attributes.md) — typed value domains for `SDL_GLAttr` set on a context.
- [Events](events.md) — event dispatcher architecture and loop integration.
