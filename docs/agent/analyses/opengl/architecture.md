# Architecture

## Decisions

- **`sak::opengl` object is context-agnostic.** The core GL API does not know the platform "context" object; it operates on the implicit per-thread *current context* contract. Context creation, selection, injection, lifecycle, and validation of a valid current context are entirely the responsibility of the lower platform layer (SDL3, SFML3, GTK4, Qt6, ...), never of `sak::opengl`. The `sak::opengl` object exposes the pseudo-global operations of the API — operations global only at current-context level (`memory_barrier`, `finish`, `viewport`, `clear`, `pixel_store`, ...) — assuming the current context is already established by the platform. It is per-context, not a fixed global singleton.
