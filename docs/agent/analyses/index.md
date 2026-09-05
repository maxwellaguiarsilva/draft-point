# Analyses

This directory contains technical analyses, deep dives, and research notes specific to the `draft-point` project.

> **Data vs. business rule:** the analysis markdowns record *decisions* and rationale, never the raw/scalar numbers the analysis produces (object counts, line counts, etc.). Scalars live in the tooling output/database; query them instead of transcribing them into the markdowns.

---

## Nodes

- **[OpenGL](opengl/index.md) [#current]:** Map and label every object of the OpenGL 4.6 core profile API to generate a strongly-typed, safe, object-oriented C++ API.
- **[Main Scope Destruction (C++)](cpp/main-scope-destruction.md):** Analysis of RAII, scope destruction order, and the dangers of `std::exit()` in C++ applications.
- **[Dimension-Agnostic Color Generation (C++)](cpp/dimension-agnostic-color-generation.md):** Proposal for a `create_color` function that maps a 1D index to N-dimensional coordinates via bit deinterleaving + spatial subdivision, using `sak` pipelines. Design/analysis in the main file; [implementation & usage](cpp/color-generation-implementation.md) split out.
- **[`value_or` Indexed Overload (C++)](cpp/value-or-indexed-overload.md):** Plan to extend `sak::pattern::value_or` for indexed containers, removing the `$1` parsing guard from `tests/adhoc/rendering/0010_sphere.cpp`. Composes with the [`to_number` niebloid](cpp/to-number-niebloid.md).
- **[`bind_front` with `std::format` (C++)](cpp/bind-format.md):** Analysis of combining `std::bind_front` with `std::format`, explaining overload resolution, `consteval` immediate escalation, reference collapsing, and working patterns.
