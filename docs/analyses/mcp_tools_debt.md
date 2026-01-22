This document is a constantly evolving line of reasoning; feel free to update it as needed.

## Technical Debt and Architectural Opportunities in Project Tools

This document outlines identified opportunities for DRY, SRP, and Visual Serenity improvements within the `tools/` directory, based on an architectural analysis conducted on January 22, 2026.

### 1. Dynamic Tool Discovery (DRY)
*   **Current State**: `tools/project_mcp.py` maintains a static `tool_registry` list. This list manually duplicates tool names, groups, descriptions, and parameter schemas (types and defaults) that are already present or enforced within the individual tool scripts (e.g., via `lib.common.ensure` or `params.get`).
*   **Opportunity**: Implement a dynamic discovery mechanism. Tool scripts could export a standardized metadata object (e.g., a `--metadata` flag or a specific Python function) that the MCP server consumes at runtime.
*   **Benefit**: Eliminates the need for manual synchronization between the server and tool implementations, reducing the risk of schema mismatches and simplifying the addition of new tools.

### 2. Python Library Standardization (SRP/DRY)
*   **Current State**: The project has a well-defined `tools/cpp/cpp_lib/` for shared C++ tool logic, but Python-specific tools (like `tools/python/code_verifier.py`) define their logic and subclasses (e.g., `formatter`) directly within the entry-point script.
*   **Opportunity**: Create a `tools/python/python_lib/` directory. Move shared logic, such as the Python-specific `formatter` class, to this library.
*   **Benefit**: Ensures that Python tools follow the same architectural pattern as C++ tools, facilitating code reuse and maintaining consistency (mimicry) across different language toolsets.

### 3. Elimination of Naming Ambiguity (Visual Serenity)
*   **Current State**: Several filenames are duplicated across different namespaces (e.g., `tools/lib/verifier.py` vs `tools/cpp/cpp_lib/verifier.py`, and `tools/lib/config.py` vs `tools/cpp/cpp_lib/config.py`).
*   **Opportunity**: Rename these files to be more descriptive and unique (e.g., `base_verifier.py` vs `cpp_verifier.py`).
*   **Benefit**: Reduces cognitive load during navigation, clarifies intent in stack traces, and adheres to the "Visual Serenity" principle by removing naming collisions that rely solely on directory context.

### 4. Generalization of Verifier Wrappers (DRY)
*   **Current State**: Functions like `run_cpp_verifier` (in `cpp_lib/verifier.py`) and `run_code_verifier` (in `python/code_verifier.py`) are essentially thin boilerplate wrappers that pass specific configurations (extensions, class names) to `lib.verifier.run_verifier`.
*   **Opportunity**: Elevate this pattern to a more generic factory or utility within `lib.verifier.py` that can be configured via a simple dictionary or parameters in the main tool script.
*   **Benefit**: Reduces the amount of repetitive "glue code" required to register a new language verifier, keeping the tool entry points focused and minimal.

## Proposed Target Architecture

To ensure consistency and allow for clean refactoring, tools should follow this 2-layer structure:

### Layer 1: Entry Point (The Machinery)
*   **Location**: `tools/<group>/<tool_name>.py`
*   **Responsibility**: Interface only.
*   **Pattern**: Uses `lib.common.run_mcp_tool` to execute a runner function imported from the library.
*   **Example**:
    ```python
    from lib.common import run_mcp_tool
    from <group>_lib.verifier import run_<language>_verifier
    if __name__ == "__main__":
        run_mcp_tool(run_<language>_verifier)
    ```

### Layer 2: Library (The Logic)
*   **Location**: `tools/<group>/<group>_lib/`
*   **Responsibility**: Domain logic and language-specific rules.
*   **Standardization**:
    *   Language-specific verifiers should be named `run_<language>_verifier`.
    *   Subclasses of `base_verifier` should be named `formatter` to keep internal symmetry.
    *   Files should be named according to their specific purpose (e.g., `cpp_verifier.py` instead of just `verifier.py`).
