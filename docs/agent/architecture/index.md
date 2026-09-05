# Architecture

This directory contains documentation regarding the domain philosophy, library design, and environment context of the `draft-point` project.

---

## Index

- **[Domain Philosophy](domain-philosophy.md):** Core priorities (DRY, DDD), rules on generated code, and language exceptions.
- **[Range Pipeline Modernization (C++26)](range-pipeline-cpp26.md):** Decisions to retire the Termux-era `sak::operator|` / `to_point` infrastructure and integrate natively with the STL via `range_adaptor_closure` and a universal `sak::ranges::to` proxy.
- **[The `sak` Library](../../../project-mcp-tools/docs/agent/sak/cpp.md):** Overview of the Swiss Army Knife library and the strategy for modernizing its C++23 STL reproductions.
- **[Environment Context](environment.md):** Details about the Arch Linux development environment and GCC 16 toolchain.