# SDL3

## Objective

- Gather information that will help build a RAII, object-oriented C++ library for SDL3, following the resources and patterns of the `sak` library.
- The resulting API lives under the `sak/sdl3` hierarchy (`include/sak/sdl3/`).
- Each SDL3 object is analyzed in its own subject markdown; this index points to them.

## Strategy

The strategy for analyzing and handling this problem is defined 100% by the owner, minutely, through specific instructions passed along the way.

## Markdown documentation rules

Rules for documenting an SDL3 API in its subject markdown:

1. Group symbols by **semantic grouping identified during the API analysis**.
2. Represent the hierarchy with bullet levels.
3. Do not include internal values or representations (e.g. numeric or hexadecimal codes).
4. Declare a common name/prefix once in an upper level and, in the following levels, use only the remaining component, without repeating the full name.
5. Do not add a description that merely paraphrases the symbol's name; include a description only when it adds a fact or value beyond the name itself. Interpreted aggressively: strip every reworking that adds no fact beyond the name, keeping only pure added facts.
6. Do not use inline lists; always one item per line (each item its own list bullet or tree node). Interpreted literally: every multi-symbol inline group is split, each member on its own bullet — this covers region/enum member enumerations, long property lists, and parameter sets alike.

## Nodes

- [Application](application.md) [#current] — RAII wrapper for the SDL3 application lifecycle.
- [Display](display.md) — RAII wrapper for an SDL3 display.
- [Window](window.md) — RAII wrapper for an SDL3 window.
- [OpenGL Context](opengl-context.md) — RAII wrapper for an SDL3 OpenGL context.
- [OpenGL Attributes](opengl-attributes.md) — typed value domains for `SDL_GLAttr` set on a context.
- [Events](events.md) — event dispatcher architecture and loop integration.
