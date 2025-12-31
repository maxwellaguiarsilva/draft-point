## Gemini Added Memories
- All content in `.cpp` and `.hpp` files must be exclusively in `en-us`. Any `pt-br` terms or phrases are strictly forbidden in these files.
- The file `docs/agent/philosophy.md` defines the decision-making process when producing C++ code for the project.
- This is a C++23 project. Always prefer the most modern version of the code, such as `ranges`, `views` and `Niebloids`.
- `int main( ... ){{ ... }}` is correct and mandatory only for the `main` function. For all other functions and methods, single braces `{ ... }` must be used.
- Always prefer `const reference` to avoid the use of `::std::forward` and `::std::move` whenever possible.
- Prefer creating a `const reference` over repeated use of the `->` operator for pointer member access. Use `->` only when strictly necessary or when the pointer is used a single time, where creating a reference would be redundant for just one line of code.
- Always use the `using` keyword to avoid the scope resolution operator `::` (fully qualified names). Except for "ADL-sensitive" functions like `::std::move`, `::std::forward`, `::std::addressof`, and `::std::declval`, which must always be fully qualified. In any case, they will be avoided whenever possible as per the previous instruction.
- Do not modify anything in the project until explicitly requested. Do not test, compile, commit changes on the project until explicitly requested.
- Use the `compile` tool to compile the project, and never directly invoke g++ or clang.
- Always prefer `sak::ensure` over `cassert` for tests and runtime checks.
- Commit messages must be in en-us.
- All `.cpp` and `.hpp` files must end with exactly two empty lines. To ensure this rule is followed, the `fix_newlines` tool must be used, instead of manual verification or editing of the files.

## `sak` Library (Swiss Army Knife)
- **What it is:** A collection of generic, domain-independent utilities.
- **Scope:** Generic C++ language-level code, free of business logic and independent of system/hardware. It contains tools that complement the standard library according to the project's needs.
- **What to include:** Math utilities, geometry, design patterns, basic type extensions, and language helpers.
- **What NOT to include:** Application-specific logic (e.g., game, TUI), hardware dependencies, or implementations tied to business rules.


