# Modernity tiers

## Objective

Separate modern, state-of-the-art Core profile objects (`canonical`) from inferior bind-to-edit (`traditional-core`) and superseded/legacy (`obsolete`) flavors, so deprecated Core functionality never leaks into newly designed greenfield C++ APIs.

## Nodes

- [canonical](canonical.md) [#current] — modern state-of-the-art API patterns (Direct State Access, modern canonical names).
- [tradicional-core](tradicional-core.md) — standard bind-to-edit core functionality that remains functional but is inferior for modern OOP design.
- [obsolete](obsolete.md) — superseded or redundant objects kept only for backwards compatibility (e.g. legacy aliased constants).

## Decisions

- **Modernity tiers for Core profile objects.** Objects within the OpenGL Core profile are categorized into three tier tags:
  - `canonical` — modern state-of-the-art API patterns (Direct State Access, modern canonical names);
  - `traditional-core` — standard bind-to-edit core functionality that remains functional but is inferior for modern OOP design;
  - `obsolete` — superseded or redundant objects kept only for backwards compatibility (e.g. legacy aliased constants).
  Rationale: prevents deprecated or obsolete flavors of the Core profile from leaking into newly designed greenfield APIs.

- **Core-profile legacy separation and critical domain judgment.** The OpenGL API splits into Core and Compatibility profiles; modern applications discard Compatibility and target Core exclusively. Within the Core profile, there is an undocumented legacy layer: functions superseded by modern paradigms (e.g. DSA, immutable storage) that Khronos never formally deprecated in `gl.xml`. Because `gl.xml` lacks formal deprecation metadata for superseded Core functions, classification into modernity tiers relies on rigorous domain analysis and critical human judgment.

- **Modern paradigms are heuristics, not literal canonical members.** Naming a modern paradigm (e.g. DSA, immutable storage) is only a heuristic of where to look for `canonical` candidates; it is never a strict or literal membership rule. Within any paradigm there can be objects already superseded by an even more modern Khronos-recommended solution, and those must be relegated to a lower tier despite belonging to the paradigm. The deciding authority is always the modernity tie-breaker (existence of a more modern recommendation), never the paradigm label.

- **Modernity tier segregation over classes.** Classes retain complete structural mappings via `class-*` owner tags, while modernity tiers (`canonical`, `traditional-core`, `obsolete`) provide the definitive filter separating modern canonical operations (Direct State Access, direct handles) from superseded bind-to-edit and legacy mechanisms.
