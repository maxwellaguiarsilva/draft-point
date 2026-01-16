#   MCP Tools Migration Analysis

##  Context and Objectives

The project is evolving its MCP (Model Context Protocol) tool architecture to a more structured and scalable model. Previously, most tools were located directly in the `tools/` root, which led to a flat organization and inconsistent naming conventions.

The new structure organizes tools into logical groups based on their domain and responsibility. This organization is reflected both in the filesystem and in the MCP server's tool naming (prefixing).

### Current Logical Groups

-   **git**: Version control operations (e.g., `git_quick_upload`).
-   **llm**: Advanced logic, statistics, and experimental tools used by the AI (e.g., `llm_statistic`, `llm_adhoc_tool`).
-   **cpp**: Development workflow, compilation, analysis, and code generation (to be migrated).

##  The New Standard

Each tool should follow these rules:

1.  **Directory Structure**: The script must be placed in a subdirectory of `tools/` that represents its group (e.g., `tools/git/`, `tools/cpp/`, `tools/llm/`).
2.  **Naming Convention**:
    -   **Filesystem**: `tools/<group>/<tool_name>.py`
    -   **MCP Tool Name**: `<group>_<tool_name>` (e.g., `git_quick_upload`, `cpp_compile`).
3.  **Encapsulation**: Scripts should use absolute imports from the `tools/` root (e.g., `from lib.common import ...`) to ensure reliability when executed via the MCP server's `PYTHONPATH`.
4.  **Registration**: In `tools/project_mcp.py`, use the `_invoke_tool( "group", "name", ... )` helper instead of the legacy invoker.

##  Migration Guidelines for `cpp` Tools

The following tools are candidates for the next migration phase under the `cpp` prefix:

-   `analyze.py` -> `tools/cpp/analyze.py` (MCP: `cpp_analyze`)
-   `compile.py` -> `tools/cpp/compile.py` (MCP: `cpp_compile`)
-   `code_verifier.py` -> `tools/cpp/code_verifier.py` (MCP: `cpp_code_verifier`)
-   `create_class.py` -> `tools/cpp/create_class.py` (MCP: `cpp_create_class`)
-   `create_test.py` -> `tools/cpp/create_test.py` (MCP: `cpp_create_test`)
-   `include_tree.py` -> `tools/cpp/include_tree.py` (MCP: `cpp_include_tree`)

### Precautions during migration:

-   **Main Function Reference**: When renaming or moving scripts, ensure the `if __name__ == "__main__":` block calls the correctly named `run_<name>` function.
-   **Header Metadata**: Update the `File:` field in the file header to reflect the new location/name.
-   **PYTHONPATH Reliance**: Always assume `tools/` is in the `PYTHONPATH`. Do not use relative imports (`..lib`).
-   **MCP Server Restart**: Any change in `tools/project_mcp.py` requires a manual restart of the MCP server to take effect.

##  Conclusion

This migration aims for "Visual Serenity" and "Intent-Based Organization" within our development tools, mirroring the philosophy applied to our C++ codebase. By grouping tools, we reduce cognitive load and make the agent's capabilities more predictable and structured.
