# How to Create and Test New MCP Tools

This guide describes the project's Model Context Protocol (MCP) tool architecture and how to use `adhoc_tool` for rapid prototyping.

## Execution Architecture

The system uses **FastMCP** as the communication interface, but adopts a decoupling strategy for greater agility:

1.  **MCP Server (`tools/project-mcp-tools`):** Acts only as a dispatcher. It defines the tool interface and uses `subprocess.run` to execute external Python scripts.
2.  **Tool Scripts (`tools/*.py`):** Contain the actual tool logic. They receive parameters via the command line (usually as a JSON string) and return the result via `stdout`.

### Advantages of this Approach

-   **Logic Hot-Reloading:** Since the MCP server calls external scripts, you can modify a script's code (e.g., `tools/file-generator.py`) and test the change immediately without needing to restart the FastMCP server.
-   **Isolation:** Errors in a tool do not bring down the main server.
-   **Simplicity:** Tool logic is pure Python, making development and debugging easier.

## Philosophy: The MCP-First Mindset

The goal of this architecture is to minimize friction and maximize the agent's high-level autonomy.

1.  **Prefer Abstractions over Manual Labor:** If a task involves repetitive steps, complex logic, or orchestrating multiple entities, do not default to manual terminal commands. Instead, think: "Can this be an MCP tool?".
2.  **Avoid the "Bash Loop" Anti-pattern:** Using the terminal to iterate over files (e.g., `for f in ...; do tool $f; done`) is a sign that the tool's interface is too low-level. While the batching example (`list[str]` instead of `str`) is a common fix, the principle applies to any orchestration: the tool should handle the complexity, not the user/agent via terminal.
3.  **The Agent as a Tool-Builder:** When you encounter a limitation in existing tools, use `adhoc_tool` to prototype a better interface. You are responsible for evolving the project's capabilities to be more robust and less error-prone.
4.  **Hot-Reloading for Rapid Iteration:** Leverage the decoupling between the MCP server and Python scripts. You can refine logic in real-time. Use this to arrive at robust, production-quality tools without constant server restarts.

## Prototyping with `adhoc_tool`

The `adhoc_tool` is a special tool designed to allow the LLM to write and test experimental logic in real-time.

### Adhoc Development Cycle

1.  **Write:** The LLM (or developer) modifies the `tools/adhoc-tool.py` file with the desired experimental logic.
2.  **Test:** The LLM calls the `adhoc_tool(params={...})` MCP tool.
3.  **Validate:** The server executes the script and returns the result for conceptual validation.

### Usage Example

If you need to test new string processing:

1.  Modify `tools/adhoc-tool.py`:
    ```python
    def run_adhoc(params):
        text = params.get("text", "")
        return f"Processed: {text.upper()}"
    ```
2.  Invoke via MCP:
    `adhoc_tool(params={"text": "hello world"})`

## Production Migration

After validating a new feature via `adhoc_tool`, the migration procedure is:

1.  **Extract Logic:** Move the tested logic to a definitive script (e.g., a new file in `tools/` or a new function in `file-generator.py`).
2.  **Register on Server:** Add the new `@mcp.tool()` to the `tools/project-mcp-tools` file.
3.  **Restart Server:** **Mandatory.** Unlike script logic, changes to the server structure (new tools or signature changes) require restarting the MCP server process so that the client (Gemini CLI) recognizes the new capabilities.