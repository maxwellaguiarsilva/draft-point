# How to Create and Test New MCP Tools

This guide describes the project's Model Context Protocol (MCP) tool architecture and how to use `llm_adhoc_tool` for rapid prototyping.

## Execution Architecture

The system uses **FastMCP** as the communication interface, but adopts a decoupling strategy for greater agility:

1.  **MCP Server (`tools/project_mcp.py`):** Acts only as a dispatcher. It defines the tool interface and uses `subprocess.run` to execute external Python scripts.
2.  **Tool Scripts (`tools/{group}/{name}.py`):** Contain the actual tool logic, organized into groups (e.g., `cpp/`, `git/`, `llm/`). They receive parameters via the command line as a JSON string and return the result via `stdout`.

### Advantages of this Approach

-   **Logic Hot-Reloading:** Since the MCP server calls external scripts, you can modify a script's code (e.g., `tools/cpp/compile.py`) and test the change immediately without needing to restart the FastMCP server.
-   **Isolation:** Errors in a tool do not bring down the main server.
-   **Validation:** Scripts use `run_mcp_tool` from `lib.common`, which ensures they are called through the MCP mechanism by checking the `MCP_CALL_LOCK` environment variable. Direct calls from the shell are detected and recorded as violations.

## Philosophy: The MCP-First Mindset

The goal of this architecture is to minimize friction and maximize the agent's high-level autonomy.

1.  **Prefer Abstractions over Manual Labor:** If a task involves repetitive steps, complex logic, or orchestrating multiple entities, do not default to manual terminal commands. Instead, think: "Can this be an MCP tool?".
2.  **Avoid the "Bash Loop" Anti-pattern:** Using the terminal to iterate over files (e.g., `for f in ...; do tool $f; done`) is a sign that the tool's interface is too low-level. The tool should handle the complexity (e.g., accepting a list of files), not the user/agent via terminal.
3.  **The Agent as a Tool-Builder:** When you encounter a limitation in existing tools, use `llm_adhoc_tool` to prototype a better interface. You are responsible for evolving the project's capabilities to be more robust and less error-prone.
4.  **Hot-Reloading for Rapid Iteration:** Leverage the decoupling between the MCP server and Python scripts. You can refine logic in real-time. Use this to arrive at robust, production-quality tools without constant server restarts.

## Prototyping with `llm_adhoc_tool`

The `llm_adhoc_tool` is a special tool designed to allow the agent to write and test experimental logic in real-time.

### Adhoc Development Cycle

1.  **Write:** Modify the `tools/llm/adhoc_tool.py` file with the desired experimental logic.
2.  **Test:** Call the `llm_adhoc_tool(params={...})` MCP tool.
3.  **Validate:** The server executes the script and returns the result for conceptual validation.

### Usage Example

If you need to test new string processing:

1.  Modify `tools/llm/adhoc_tool.py`:
    ```python
    def run_adhoc_tool( params ):
        text = params.get( "text", "" )
        return  f"processed: {text.upper( )}"
    ```
2.  Invoke via MCP:
    `llm_adhoc_tool(params={"text": "hello world"})`

## Production Migration

After validating a new feature via `llm_adhoc_tool`, the migration procedure is:

1.  **Extract Logic:** Move the tested logic to a definitive script in the appropriate group directory (e.g., `tools/cpp/my_new_tool.py`).
2.  **Register on Server:** Add the new `@mcp.tool()` to the `tools/project_mcp.py` file.
3.  **Restart Server:** **Mandatory.** Unlike script logic, changes to the server structure (new tools or signature changes) require restarting the MCP server process so that the client (Gemini CLI) recognizes the new capabilities. Ask the user to restart the server after registration.