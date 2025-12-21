## Gemini Added Memories
- This is a C++23 project. Always prefer the most modern version of the code, such as `ranges`, `views` and `Niebloids`.
- `int main( ... ){{ ... }}` is correct and mandatory only for the `main` function. For all other functions and methods, single braces `{ ... }` must be used.
- After producing any C++ code, validate it using the guidelines in the document `docs/agent/coding-style/cpp.md`
- Do not modify anything in the project until explicitly requested. Do not test or compile the project until explicitly requested.
- Use the `project-builder` command to compile the project, and never directly invoke g++ or clang.
- The `docs/agent/` directory contains markdown files with specific instructions for this programming assistant.
- When a user makes a request, check if there is a relevant markdown file in `docs/agent/` that provides instructions for that request. If a relevant file is found, I will load its content into the conversation context to guide my actions.
- Commit messages must be in en-us.

