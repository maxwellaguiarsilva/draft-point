# Figure Demo

This directory documents the figures demo declared in `source/figures_demo.cpp`, together with the pipeline pieces that demo consumes.

## Current Goal

- Render the figures demo in `source/figures_demo.cpp` with `data/shader/figures.glsl` and the figure evaluators under `data/shader/figures/`.

---

## Nodes

- **[SSBO layout](ssbo-reflection.md):** Flat float exchange between C++ payloads and GLSL blocks, the `is_flat` validation, `shader::storage_buffer`, and the reasons the typed `std430` alternative is rejected.
- **[Request log](log.md):** Laconic record of every request made for this subject, each one carrying a one-word status.

## Request log contract

- `log.md` records every request the user makes for this subject, appended in the order the requests arrive.
- Each entry is one bullet stating the request laconically plus one status word.
- The list carries no numbers: entries use plain bullets, and neither the marker nor the text restates a sequence number.
- The status word is one of `pending`, `doing`, `done`, `blocked`, or `dropped`.
- An entry is updated rather than duplicated when its request advances, so the log holds one bullet per request.
- The log is not a plan, an analysis, or a justification; it holds the request and the status word only.
