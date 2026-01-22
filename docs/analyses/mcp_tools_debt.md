This document is a constantly evolving line of reasoning; feel free to update it as needed.


## IDENTIFIED DRY DEBTS ✨

*   **Boilerplate and Dynamic Dispatch in `tools/project_mcp.py`**:
    *   **Problem**: Every tool registration repeats the `@mcp.tool` decorator, the `_invoke_tool` call, and the use of `locals().copy()` to capture arguments. This creates a high maintenance cost; changing how tools are invoked (e.g., adding a new environment variable or changing the python interpreter) requires editing every single function.
    *   **Propose**: Move tool registration to a "Data over Machinery" approach. Define a central dictionary or list of tuples containing `(group, name, description, parameters)` and use a loop to dynamically register these tools in the FastMCP instance.

*   **Hardcoded Formatting Constants and Rules**:
    *   **Problem**: The project's "Visual Serenity" standards, such as the `newline_3` constant (`\n\n\n`) and regex rules for keyword spacing (e.g., `return` spacing), are redeclared across `lib/verifier.py`, `cpp_lib/verifier.py`, and `python/code_verifier.py`.
    *   **Propose**: 
        *   Centralize universal formatting constants and regex patterns (like `newline_3`) in a `common_rules` dictionary within `tools/lib/verifier.py`.
        *   Ensure that language-specific verifiers (C++, Python) compose their logic by extending this central rule set, rather than redeclaring patterns.

*   **Redundant Path Calculation in Creation Tools**:
    *   **Problem**: `tools/cpp/create_class.py` and `tools/cpp/create_test.py` both implement logic to split a hierarchy string (e.g., "game/player") and map it to the physical directory structure (include, source, or tests).
    *   **Propose**: Centralize path intelligence in `tools/cpp/cpp_lib/project_model.py`. The model should provide a method like `get_path_for_hierarchy(hierarchy, file_type)` that returns the absolute or relative path based on the project's configuration, ensuring that any change to the folder structure only needs to be updated in one place.

---

### Contextual Note
Architectural debts—particularly violations of DRY and SRP—serve as concrete examples of the pitfalls described in [llm interaction principles](llm_interaction_principles.md). 
These issues often arise when the LLM prioritizes the "direct delivery" of a feature over architectural refinement. Preventing or resolving such debts requires moving beyond "delivery-focused" implementation towards a more principled, iterative approach that values small, certain steps and structural integrity over quick, context-limited fixes.
