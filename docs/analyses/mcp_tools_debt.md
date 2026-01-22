# Technical Debt and Architectural Opportunities in Project Tools

This document outlines identified opportunities for DRY, SRP, and Visual Serenity improvements within the `tools/` directory.

## Current Technical Debt

### 1. Python Library Standardization (SRP/DRY)
*   **Current State**: Python-specific tools (like `tools/python/code_verifier.py`) define their logic and subclasses (e.g., `formatter`) directly within the entry-point script.
*   **Opportunity**: Create a `tools/python/python_lib/` directory. Move shared logic, such as the Python-specific `formatter` class, to this library.
*   **Benefit**: Ensures that Python tools follow the same architectural pattern as C++ tools, facilitating code reuse and maintaining consistency (mimicry) across different language toolsets.

### 2. Elimination of Naming Ambiguity (Visual Serenity)
*   **Current State**: Several filenames are duplicated across different namespaces (e.g., `tools/lib/verifier.py` vs `tools/cpp/cpp_lib/verifier.py`, and `tools/lib/config.py` vs `tools/cpp/cpp_lib/config.py`).
*   **Opportunity**: Rename these files to be more descriptive and unique (e.g., `base_verifier.py` vs `cpp_verifier.py`).
*   **Benefit**: Reduces cognitive load during navigation, clarifies intent in stack traces, and adheres to the "Visual Serenity" principle.

## Completed Refactorings

### 1. Dynamic Tool Discovery (DRY)
*   **Status**: Successfully implemented.
*   **Outcome**: The MCP server in `tools/project_mcp.py` now dynamically discovers tools by inspecting the `tools/` directory, extracting metadata (descriptions and parameters) from function signatures and docstrings. The static `tool_registry` was eliminated.

### 2. Generalization of Verifier Wrappers (DRY)
*   **Status**: Successfully implemented.
*   **Outcome**: The `lib.verifier` module now provides a `base_verifier` class and a `run_verifier` utility that handles the common boilerplate for all language-specific verifiers.

## Target Architecture

Tools must follow this 2-layer structure:

### Layer 1: Entry Point (The Machinery)
*   **Location**: `tools/<group>/<tool_name>.py`
*   **Responsibility**: Interface only.
*   **Pattern**: Uses `lib.common.run_mcp_tool` to execute a runner function defined in the same file or imported from the library.

### Layer 2: Library (The Logic)
*   **Location**: `tools/<group>/<group>_lib/`
*   **Responsibility**: Domain logic and language-specific rules.