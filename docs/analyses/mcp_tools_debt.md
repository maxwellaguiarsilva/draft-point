This document is a constantly evolving line of reasoning; feel free to update it as needed.


## RESOLVED DRY DEBTS ✨

*   **Boilerplate and Dynamic Dispatch in `tools/project_mcp.py`**:
    *   **Status**: RESOLVED
    *   **Solution**: Tool registration was refactored to use a "Data over Machinery" approach. Tools are now defined in a central list and dynamically registered in a loop, significantly reducing boilerplate and maintenance overhead.

*   **Hardcoded Formatting Constants and Rules**:
    *   **Status**: RESOLVED
    *   **Solution**: Universal formatting constants (like `newline_3`) were centralized in a `common_rules` property within `tools/lib/verifier.py`. Language-specific verifiers (C++, Python) now inherit and reuse these central rules.

*   **Redundant Path Calculation in Creation Tools**:
    *   **Status**: RESOLVED
    *   **Solution**: Path intelligence was centralized in `tools/cpp/cpp_lib/project_model.py` via the `get_path_for_hierarchy` method. `create_class.py` and `create_test.py` were updated to use this single source of truth for folder structures and file mapping.