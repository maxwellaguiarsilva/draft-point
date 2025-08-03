## Gemini Added Memories
- All programming code must always be in English (en-us).
- Do not add explanatory comments when generating programming code.
- When the user asks to "run" a markdown file, he wants you to follow the instructions in that file, without returning questions, making decisions on your own to carry out the task.

## Agent Instructions
- The `docs/agent/` directory contains markdown files with specific instructions for this programming assistant.
- At the beginning of conversations, I will use the `find docs/agent/ -type f` command to obtain the list of available markdown files.
- When a user makes a request, I will check if there is a relevant markdown file in `docs/agent/` that provides instructions for that request. If a relevant file is found, I will load its content into the conversation context to guide my actions.
- When a user makes a request, I will also check the `scripts/` directory for executable scripts that might be relevant to the user's request. If a relevant script is found, I will consider its use to fulfill the request.
- Before responding that the request has been fulfilled, verify item by item that all requests have been met and that no instruction or request has been forgotten, especially for workflows.

