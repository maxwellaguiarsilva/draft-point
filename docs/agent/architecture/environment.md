# Environment Context

This document outlines the development environment context for the `draft-point` project.

---

## Environment Transition

The development environment has transitioned from a highly constrained setup (Termux on Android and Windows 11) to a modern, fully-featured **Arch Linux** environment.

### Key Environmental Characteristics:

* **Compiler & Toolchain:** Arch Linux provides an extremely modern version of GCC (GCC 16), which offers complete, robust support for the **C++23** standard library, including advanced ranges and views. The project compiles with `g++` and no experimental flags are required.
* **Process Execution:** Spawning subprocesses, managing file paths, and running parallel builds are significantly faster and more reliable on Arch Linux compared to Termux or Windows.
* **Project Separation:** `draft-point` (the C++ application and SAK library) and `project-mcp-tools` (the Python-based developer tools framework) are completely separate repositories. `project-mcp-tools` is cloned as a sibling directory.