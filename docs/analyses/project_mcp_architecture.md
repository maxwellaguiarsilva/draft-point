# Project-Mcp-Tools System Architecture and Operation

This document describes the technical architecture, components, and workflows of the project's Model Context Protocol (MCP) tool system.

## 1. Design Philosophy: Gold Standard

The system is transitioning from a coupled model to an agnostic and idiomatic pattern called **Gold Standard**. The goal is for the MCP server to be merely an orchestrator that does not know the internal implementation of the tools.

### 1.1. Gold Standard Tool Specification
To be considered Gold Standard, a tool must strictly follow these requirements:

1.  **Naming**: The file must use `snake_case` (e.g., `quick_upload.py`). This allows tools to import each other natively via Python.
2.  **Header**: It must contain the `shebang` (`#!/usr/bin/python3`) followed by the project's standard license header and the `File:` field with the correct filename.
3.  **Entry Point**: It must use the `run_main` function from the `lib.common` module.
4.  **Main Function**: It must be named `run_<tool_name>` (e.g., `run_quick_upload`) and accept a single dictionary parameter (`params`).
5.  **Return**: For success, it must return only the informative content string. The responsibility for indicating "success" or "failure" at the interface level belongs to the Dispatcher (`project-mcp.py`).
6.  **Error Handling**: For failures, the tool must raise a standard `Exception` with a clear, descriptive message. This message must *not* include prefixes like "error:" or "failure:", as labeling is the Dispatcher's responsibility. This allows `lib.common` to capture the error, print the clean message to `stderr`, and exit with status 1, signaling the failure to the orchestrator.
6.  **Process Decoupling**: If a tool needs another (e.g., `quick_upload` calling `agent_statistic`), it must perform a native `import` of the `run_` function instead of triggering a `subprocess`.

### 1.2. The Dispatcher (MCP Server)
Located in `tools/project-mcp.py`, it uses the `FastMCP` library.
*   **Agnostic**: For Gold Standard tools, it uses the `_invoke_tool` function, which assumes the MCP tool name is identical to the `.py` filename in `tools/`.
*   **Legacy**: Tools that have not yet been migrated use `_legacy_run_and_format` and depend on the `_special_tool_config` dictionary to map subcommands and old filenames (`kebab-case`).

---

## 2. Components and Migration Status

### 2.1. Gold Standard Tools (Migrated)
| Tool | File | Description |
| :--- | :--- | :--- |
| `adhoc_tool` | `adhoc_tool.py` | Executes experimental logic and prototyping. |
| `quick_upload` | `quick_upload.py` | Git cycle (pull, add, commit, push) and statistics recording. |
| `agent_statistic` | `agent_statistic.py` | Recording and querying agent behavior metrics. |

### 2.2. Legacy Tools (Pending Migration)
These tools still use the `kebab-case` pattern, manual subcommands, and do not follow the `run_` function structure.

*   **`file-generator.py`**: (`create_class`, `create_test`). Boilerplate and template generator.
*   **`code-verifier.py`**: (`verify_formatting`). Style Hard Rules validator.
*   **`project-builder.py`**: (`compile`, `analyze`). Build, cppcheck, and formatting orchestrator.

---

## 3. Gold Standard Workflows

### 3.1. Execution via MCP
The execution workflow follows this path:
1.  **Call**: `mcp.quick_upload(message="...")` in `project-mcp.py`.
2.  **Dispatch**: `_invoke_tool("quick_upload", {"message": "..."})`.
3.  **Shell**: `python3 tools/quick_upload.py '{"message": "..."}'`.
4.  **Logic**: `run_main(run_quick_upload)` processes the JSON and executes the task.
5.  **Output**: The tool's clean return is encapsulated by the MCP success label.

### 3.2. Integration between Tools
A Gold Standard tool can invoke another without creating new processes:
```python
from agent_statistic import run_agent_statistic

def run_my_tool(params):
    # ... logic ...
    return run_agent_statistic({"name": "success"})
```

---

## 4. Implementation Guide for New Tools

When creating a new tool, use the following basic template:

```python
#!/usr/bin/python3

# [Standard License Header]
# File: tool_name

from lib.common import run_main

def run_tool_name(params):
    # Logic here
    if not params.get( "required_key" ):
        raise Exception( "missing required_key parameter" )
    
    return "result"

if __name__ == "__main__":
    run_main(run_tool_name)
```

---

## 5. Maintenance Notes
*   **Sanitization**: After migrating logic from `adhoc_tool.py` to a definitive script, `adhoc_tool.py` must be cleaned.
*   **Refactoring**: The long-term goal is to eliminate `_special_tool_config` from `project-mcp.py` as all scripts are migrated to the Gold Standard `snake_case` pattern.
