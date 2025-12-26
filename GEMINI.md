## Gemini Added Memories
- This is a C++23 project. Always prefer the most modern version of the code, such as `ranges`, `views` and `Niebloids`.
- `int main( ... ){{ ... }}` is correct and mandatory only for the `main` function. For all other functions and methods, single braces `{ ... }` must be used.
- Always prefer `const reference` to avoid the use of `::std::forward` and `::std::move` whenever possible.
- Always use the `using` keyword to avoid the scope resolution operator `::` (fully qualified names). Except for "ADL-sensitive" functions like `::std::move`, `::std::forward`, `::std::addressof`, and `::std::declval`, which must always be fully qualified. In any case, they will be avoided whenever possible as per the previous instruction.
- Do not modify anything in the project until explicitly requested. Do not test or compile the project until explicitly requested.
- Use the `compile` tool to compile the project, and never directly invoke g++ or clang.
- Commit messages must be in en-us.

