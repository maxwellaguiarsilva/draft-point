# TUI Binaries and User Feedback

When testing TUI-based binaries (e.g., raytracing tests that need a terminal), use `tmux split-window` to create a pane alongside the conversation. The user will see the output immediately in the new pane. After launching, ask the user if the visual result looks correct — the agent cannot see the TUI output directly. Always keep the working directory set, so the binary path stays consistent.

Example:

```
tmux split-window -h "./dist/0009_raytracing_primitives; read -p 'press enter to close'"
```