# Draft Point

This project is a didactic exploration of modern C++23, focused on **Semantic Elevation**. The goal is to move away from low-level "machine instructions" and instead write code that serves as a clear statement of intent.

## The `sak` Library (Swiss Army Knife)

The core of this project is the `sak` library. It is designed as a collection of generic, domain-independent utilities covering mathematics, geometry, and design patterns. 
- **Domain Agnostic:** It contains no business logic or hardware dependencies.
- **Modern Paradigms:** It leverages C++23 features such as `ranges`, `views`, and custom `Niebloids` to reduce visual noise and promote functional composition.

## Project Highlights

### Local MCP System & Tools
The project implements a **Dispatcher-Script** architecture for its Model Context Protocol (MCP) tools, providing a powerful automation suite for the development agent.

| Tool | Description |
| :--- | :--- |
| `create_class` | Generates a C++ class pair (`.hpp`/`.cpp`) with automatic namespaces, license headers, and header guards. |
| `create_test` | Creates structured tests mapping the source hierarchy or adhoc experiments in `tests/adhoc/`. |
| `compile` | Orchestrates incremental and parallel builds, analyzing dependencies to minimize compilation time. |
| `analyze` | Performs exhaustive static analysis (`cppcheck`) and applies automatic formatting fixes to the codebase. |
| `verify_formatting` | Validates adherence to strict project style rules (newline patterns, bracket spacing, return formatting). |
| `agent_statistic` | Tracks behavioral metrics and recurring errors to refine agent performance and avoid past mistakes. |
| `adhoc_tool` | Provides a sandbox for executing experimental logic without modifying the core MCP server. |
| `quick_upload` | Automates the git workflow (pull, add, commit, and push) for rapid iteration. |

### Generic Multidimensional Point
The `sak::point` class is a high-level abstraction for N-dimensional arithmetic.
- **Range Integration:** It seamlessly converts from and to ranges (using `to_point`), allowing for expressive data pipelines.
- **Lazy Evaluation:** Supports pipe operators (`|`) for transformations, enabling complex geometric operations without immediate execution overhead.
- **Semantic Operators:** Arithmetic operations work intuitively across dimensions, treating the point as a first-class mathematical entity.

### Generic Pattern Dispatcher
A thread-safe, non-intrusive `dispatcher` implementation.
- **Weak Linkage:** Uses `std::weak_ptr` to store listeners, preventing circular dependencies and memory leaks.
- **Automatic Cleanup:** Identifies and removes expired listeners during event dispatching.
- **Robustness:** Handles exceptions during method invocation, returning a `std::expected` with detailed failure reports.

### Half-Block Pseudo-Pixels
The `tui::renderer` employs a "half-block" rendering technique to overcome terminal resolution limits.
- **Resolution Doubling:** By using the Unicode character `▀` (Upper Half Block), the renderer can treat the foreground and background of a single character cell as two distinct vertical pixels.
- **Optimization:** Performs differential updates, only sending ANSI escape codes when a "pixel" actually changes.

### Shadertoy in the Terminal
The `shadertoy` class brings the concept of fragment shaders to the TUI.
- **Functional Rendering:** Accepts a "shader" function (a mapping from coordinates and time to a color point).
- **Time-Driven:** Manages the high-resolution clock and frame-rate limiting for smooth, real-time visual effects.

## Project Philosophy

This codebase adheres to "Visual Serenity". We prefer `const&` to avoid pointer noise, use keywords like `and`/`or` for readability, and enforce strict spacing rules to ensure the code is as pleasant to read as it is to execute.
