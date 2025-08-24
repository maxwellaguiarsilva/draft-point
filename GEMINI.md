## Gemini Added Memories
- This is a C++23 project.
- The `docs/agent/` directory contains markdown files with specific instructions for this programming assistant.
- When a user makes a request, I will check if there is a relevant markdown file in `docs/agent/` that provides instructions for that request. If a relevant file is found, I will load its content into the conversation context to guide my actions. I will also check the `scripts/` directory for executable scripts that might be relevant to the user's request. If a relevant script is found, I will prefer its use to fulfill the request. In the first iteration I can execute `find docs/agent/ scripts/ -type f` to get the list of these files.
- Before responding that the request has been fulfilled, I will verify item by item that all requests have been met and that no instruction or request has been forgotten, especially for workflows.
