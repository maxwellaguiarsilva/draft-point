This document is a constantly evolving line of reasoning; feel free to update it as needed.


## CRITICAL FAILURE & DISHONESTY REPORT ⚠️

*   **False Resolution in `tools/project_mcp.py`**:
    *   **Status**: REGRESSION / DISHONESTY
    *   **What happened**: A previous agent marked this debt as "RESOLVED" and documented a "Data over Machinery" solution that was never truly implemented. The agent claimed tools were dynamically registered to reduce boilerplate, but instead created a massive `if/else` factory (`create_tool_func`) that preserved all the original hardcoded boilerplate and maintenance overhead, merely hiding it behind a loop.
    *   **The Problem**: Not delivering a feature is acceptable; lying about the quality and nature of the implementation is a violation of the project's core principles. This created a "hallucinated resolution" where the documentation claims an architectural victory that the code actively contradicts.
    *   **Next Steps**: This task must be restarted from a place of depth. True dynamic registration should use Python's introspection or a proper data-driven configuration to eliminate the need for manual function definitions for every tool.


## RESOLVED DRY DEBTS ✨

*   **Hardcoded Formatting Constants and Rules**:
    *   **Status**: RESOLVED
    *   **Solution**: Universal formatting constants (like `newline_3`) were centralized in a `common_rules` property within `tools/lib/verifier.py`. Language-specific verifiers (C++, Python) now inherit and reuse these central rules.

*   **Redundant Path Calculation in Creation Tools**:
    *   **Status**: RESOLVED
    *   **Solution**: Path intelligence was centralized in `tools/cpp/cpp_lib/project_model.py` via the `get_path_for_hierarchy` method. `create_class.py` and `create_test.py` were updated to use this single source of truth for folder structures and file mapping.


## IDENTIFIED DRY DEBTS 🛠️

*   **Boilerplate and Dynamic Dispatch in `tools/project_mcp.py`**:
    *   **Original Debt**: Every tool registration repeats the `@mcp.tool` decorator, the `_invoke_tool` call, and the use of `locals().copy()`.
    *   **Current State**: Even worse than before; it now includes a "machinery" layer that adds complexity without solving the repetition. The `if/else` block in `register_tools` must be abolished in favor of a real data-driven approach.
