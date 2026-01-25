# How to Create and Test New MCP Tools

This guide describes the project's Model Context Protocol (MCP) tool architecture, which favors dynamic discovery and decoupling for rapid development.

## Execution Architecture

The system uses **FastMCP** as the communication interface, but employs a dispatching strategy to external scripts:

1.  **Dynamic Discovery Server (`tools/project_mcp_server.py`):** Scans the `tools/` directory for subdirectories (groups) and Python scripts. It automatically registers each script as an MCP tool.
2.  **Tool Scripts (`tools/{group}/{name}.py`):** Contain the actual logic. They are executed as independent processes.
3.  **Metadata Extraction:** The server inspects the `run_{name}` function signature and docstring in each script to define the MCP tool's parameters and description.

### Advantages

-   **Hot-Reloading:** Logic changes within a script take effect immediately without restarting the MCP server.
-   **Strong Typing:** MCP parameters are derived directly from Python type hints and default values.
-   **Security:** The `MCP_CALL_LOCK` environment variable ensures tools are only executed through the authorized MCP interface. Direct shell execution is blocked and recorded as a violation.

## Tool Development Workflow

### 1. Create the Script
Place your new tool in the appropriate group directory. The file name determines the tool name suffix.
Example: `tools/group/my_tool.py` will be registered as `group_my_tool`.

### 2. Implement the Logic
The script must follow these conventions:
-   Define a function named `run_{name}` (e.g., `run_my_tool`).
-   Use type hints and default values for parameters.
-   Provide a docstring for the tool description.
-   Return a `str`.
-   Call `run_mcp_tool` in the `if __name__ == "__main__":` block.

```python
from lib.common import run_mcp_tool

def run_my_tool( name: str, count: int = 1 ) -> str:
    """description of what the tool does
    it will be displayed in the mcp interface"""
    return  f"hello {name}, " * count

if __name__ == "__main__":
    run_mcp_tool( run_my_tool )
```

### 3. Register and Restart
Structural changes require restarting the MCP server (ask the user to restart if running in the Gemini CLI).
**Restart is mandatory when:**
-   Creating a new tool file.
-   Renaming a file or the `run_` function.
-   Changing the function signature (adding/removing parameters, changing types).
-   Updating the docstring.

## Prototyping with `llm_adhoc_tool`

For experimental logic, use `tools/llm/adhoc_tool.py`. This avoids the need for constant registration/restarts during the early stages of a new feature.

1.  Modify `tools/llm/adhoc_tool.py` with your experimental code.
2.  Invoke it via the `llm_adhoc_tool` MCP command.
3.  Once the logic is stable, migrate it to a permanent script following the steps above.

## Best Practices

1.  **Reuse Utilities:** Use `tools/lib/common.py` for common tasks like `ensure`, `validate_params`, `create_process`, and `get_cpu_count`.
2.  **Explicit Imports:** The `PYTHONPATH` is set to include `tools/` and the script's own group directory. You can import group-specific libraries (e.g., `from lib.common import ensure`).
3.  **No Chitchat:** Tools should return concise, actionable information. Use `print` within the `run_` function (which `run_mcp_tool` captures and returns).
4.  **Error Handling:** Use `ensure` with clear messages. `run_mcp_tool` will catch these and return them via `stderr`.


