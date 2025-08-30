## Gemini Added Memories
- This is a C++23 project.
- I will always use the "project-builder" command to compile the project, and I will never directly invoke g++.
- The `docs/agent/` directory contains markdown files with specific instructions for this programming assistant.
- When a user makes a request, I will check if there is a relevant markdown file in `docs/agent/` that provides instructions for that request. If a relevant file is found, I will load its content into the conversation context to guide my actions.
- Before performing any procedure, or execute tools that generates or modifies programming code, I will consult the project's coding style rules,
  to ensure that the parameters passed to these tools for code generation will obey the guidelines.
- Before responding that the request has been fulfilled, I will verify item by item that all requests have been met and that no instruction or request has been forgotten, especially for workflows.
