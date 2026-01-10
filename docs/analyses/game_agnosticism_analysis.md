# Game Class Agnosticism Analysis

The following analysis investigates whether the `game::game` class is agnostic to the terminal/TUI subsystem, specifically considering a potential migration to a different renderer like `opengl_renderer`.

## Findings

1.  **Interface Abstraction**: The `game` class interacts with the renderer through a `renderer` reference. Methods used (`size`, `clear`, `draw`, `print`, `refresh`) are semantically generic enough for most graphical systems.
2.  **Coordinate System**: The use of `point`, `line`, and `rectangle` from the `tui` namespace assumes a coordinate system. While these are currently mapped to characters in the terminal, they could represent pixels or normalized coordinates in OpenGL.
3.  **Physical Coupling**:
    *   **Includes**: `game.hpp` directly includes `<tui/terminal.hpp>` and `<tui/renderer.hpp>`.
    *   **Namespace**: It heavily uses the `tui` namespace symbols.
    *   **Initialization**: The `renderer` currently requires a `terminal&` in its constructor, though `game` only holds a reference to the `renderer`.
4.  **Terminal-Specific Calls**:
    *   `renderer::read_char()`: This is the most direct dependency on terminal input.
    *   `renderer::clear_screen()`: Used at the end of `run()`, its behavior is highly specific to terminal cleanup.

## Conclusion

The `game` class is **semantically agnostic** but **physically coupled**. To achieve true independence:
- The `renderer` should be an abstract interface.
- Terminal-specific headers should be removed from `game.hpp` and replaced with forward declarations or a base class header.
- Input handling (`read_char`) should be abstracted.

Currently, renaming `terminal_listener` to `renderer_listener` was a correct step towards better semantic alignment.
