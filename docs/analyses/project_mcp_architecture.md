# Project-Mcp-Tools Architecture and Operation

This document describes the technical architecture, components, and workflows of the project's Model Context Protocol (MCP) tool system.

## 1. Tool Specification

A tool follows these requirements:

1.  **Naming**: Files use `snake_case` (e.g., `quick_upload.py`). Tools import each other via Python's `import` mechanism.
2.  **Header**: Contains the `shebang` (`#!/usr/bin/python3`), the project license header, and the `File:` field with the filename.
3.  **Entry Point**: Utilizes the `run_main` function from the `lib.common` module.
    *   **JSON-CLI Bridge**: `run_main` receives the first CLI argument (`sys.argv[1]`) as a JSON string.
    *   **Parsing**: The string is decoded and passed to the tool's main function as a `params` dictionary.
4.  **Main Function**: Named `run_<tool_name>` (e.g., `run_quick_upload`), accepting a single `params` dictionary.
5.  **Return Contract**: Returns a string containing the result content. The Dispatcher (`project_mcp.py`) handles the interface-level status.
6.  **Error Handling**: Failures are handled by throwing an `Exception` (via the `ensure` utility). `lib.common` captures the exception, prints the message to `stderr`, and exits with status 1.
7.  **Process Decoupling**: Tools interact via native `import` of the `run_` function.

### 1.2. Process Decoupling Rules
The system architecture separates process execution types:
*   **External Binaries**: Handled via `subprocess` (e.g., `git`, `g++`, `clang-format`, `cppcheck`).
*   **Internal Logic**: Python scripts within the project do not invoke each other via `subprocess`. Communication is handled through `import`.

---

## 2. Core Components

### 2.1. The Dispatcher (project_mcp.py)
The MCP server, utilizing `FastMCP`, acts as a dispatching layer.
*   **Agnosticism**: It uses the `_invoke_tool` function to execute scripts in `tools/`.
*   **Command Mapping**: Maps MCP commands to scripts. A script handles multiple commands by dispatching based on parameters.

### 2.2. The Project Engine (lib/project_core.py)
Responsible for file mapping, dependency graphs, and build orchestration.
*   **State Management**: Maintains an internal state of the filesystem and dependency metadata.
*   **Parallelism**: Uses `ThreadPoolExecutor` for compilation and analysis.
*   **Thread-Safety**: 
    *   **Output Synchronization**: Employs a `threading.Lock` to serialize print statements.
    *   **Interruption Control**: Uses `threading.Event` to halt worker threads when an error is detected.
*   **Fail-Fast Rule**: Execution stops at the first encountered error.

### 2.3. Support Libraries (`tools/lib/`)
*   **`common.py`**: Contains the `run_main` entry point and the `ensure` validation utility.
*   **`metadata_provider.py`**: Centralizes project data (Git info, licenses, dates) to prevent circular dependencies.
*   **`template_engine.py`**: Library for rendering file templates.

---

## 3. Tooling Ecosystem

| Tool | Script | Description |
| :--- | :--- | :--- |
| `analyze` | `analyze.py` | Orchestrates `cppcheck` and `code_verifier`. |
| `compile` | `compile.py` | Handles building and linking via `project_core`. |
| `code_verifier` | `code_verifier.py` | Validates and applies formatting rules. |
| `create_class` | `create_class.py` | Creates a new C++ class with .hpp and .cpp files. |
| `create_test` | `create_test.py` | Creates a new C++ test file. |
| `quick_upload` | `quick_upload.py` | Manages the Git lifecycle. |
| `agent_statistic`| `agent_statistic.py`| Records and retrieves behavioral metrics. |
| `include_tree` | `include_tree.py` | Analyzes C++ include hierarchies. |
| `adhoc_tool` | `adhoc_tool.py` | Entry point for experimental logic. |

---

## 4. Implementation Guide for New Tools

New tools utilize the following structure:

```python
#!/usr/bin/python3

# [Standard License Header]
# File: tool_name

from lib.common import run_main, ensure

def run_tool_name(params):
    # 1. Parameter validation
    ensure("required_param" in params, "Missing 'required_param'")
    
    # 2. Logic implementation
    # ...
    
    return "Operation result: [details]"

if __name__ == "__main__":
    run_main(run_tool_name)
```

### 4.1. Integration Patterns
*   Tools needing logic from another script use `import` for the corresponding `run_` function.
*   Tools requiring project-wide file mapping use `project_core`.
*   Tools requiring project metadata use `metadata_provider`.

---

## 5. Workflows

### 5.1. The "Analyze" Workflow
1.  **Mapping**: `project_core` scans the filesystem.
2.  **Static Analysis**: `project_core` dispatches threads to run `cppcheck`.
3.  **Formatting Validation**: `analyze.py` calls `code_verifier.run_code_verifier` natively.
4.  **Reporting**: The first violation triggers `ensure`, interrupting the process and returning the error.

### 5.2. The "Compile" Workflow
1.  **Graph Generation**: `project_core` builds a dependency graph.
2.  **Incremental Build**: Recompiles modified files or those with changed dependencies.
3.  **Parallel Execution**: Compilation units are processed in a thread pool.
4.  **Linking**: Final binary is linked if compilation units succeed.