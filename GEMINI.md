## Gemini Added Memories
- This is a C++23 project. Always prefer the most modern version of the code, such as `ranges`, `views` and `Niebloids`.
- `int main( ... ){{ ... }}` is correct and mandatory only for the `main` function. For all other functions and methods, single braces `{ ... }` must be used.
- Always use the `using` keyword to avoid the scope resolution operator `::` (fully qualified names), except for "ADL-sensitive" functions like `::std::move`, `::std::forward`, `::std::addressof`, and `::std::declval`, which must always be fully qualified.
- Do not modify anything in the project until explicitly requested. Do not test or compile the project until explicitly requested.
- Use the `project-builder` command to compile the project, and never directly invoke g++ or clang.
- Commit messages must be in en-us.

