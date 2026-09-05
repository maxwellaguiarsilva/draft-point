# OpenGL

## Objective

- Map and label every object of the OpenGL 4.6 core profile API, to generate a strongly-typed, safe, object-oriented C++ API.
- Steer the greenfield API to obligatorily direct the programmer toward the most modern Khronos-recommended OpenGL usage.
- Even within the Core profile, residue remains of what is no longer recommended because a more modern alternative exists; the generated API must not expose such residue as canonical.

## Decisions

- **Modernity tie-breaker (canonical) = presence of a more modern Khronos recommendation.** The single deciding criterion for whether a Core-profile object is `canonical` is whether Khronos recommends a more modern alternative for the operation it performs. If such an alternative exists, the object is not `canonical` — it is relegated to a lower modernity tier (`traditional-core` for bind-to-edit delivery, `obsolete` for superseded/redundant objects). If no modern alternative exists, the object stays `canonical`. This criterion is the heart of the project intent and resolves all otherwise-ambiguous modernity memberships.

## Strategy

The strategy for analyzing and handling this problem is defined 100% by the owner, minutely, through specific instructions passed along the way.

## Tooling

Python work for this analysis lives in the `uv` project [`opengl-tag/`](opengl-tag/), [`README.md`](opengl-tag/README.md).
Accessing this folder without reading the `README.md` file is prohibited.

## Nodes

- [Naming](naming.md) — snake_case convention, original-name column, atomic tags, vector-type-suffix tokenization.
- [Tooling](tooling.md) — uv/SQLite, automated pipeline, category tags, `gl.xml` as labeling source.
- [Data model](data-model.md) — flat `tag`+`rule`+`operation`, bitmask sets derived.
- [Tagging](tagging.md) — `group`/`class` tags, class ownership, enum scoping.
- [Modernity tiers](tiers.md) [#current] — `canonical`/`traditional-core`/`obsolete` classification.
- [Architecture](architecture.md) — `sak::opengl` context-agnostic.
- [`cpp-install-glad`](cpp-install-glad.md) — how the glad 4.6 core loader and its snake_case C++ wrapper are generated.
