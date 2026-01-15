# Diagnostic Report: Project Tree Refactoring (v2)

This document analyzes the requirements and architecture for the new `project_tree` component, designed to decouple project mapping from operational tools (`project_core`, `include_tree`).

## 1. Current State Analysis

### 1.1. Acoupling in `project_core.py`
Currently, `project_core` serves two distinct purposes:
- **Project Mapping**: Scanning files, parsing `#include` directives, and building the internal dependency graph (`hierarchy_items`).
- **Build Orchestration**: Generating compiler flags, managing thread pools for compilation, and invoking external binaries (clang++, cppcheck).

This mix makes it difficult to use the project "map" for non-build tasks (like visualization) without dragging in build-specific logic and configurations.

### 1.2. Redundancy in `include_tree.py`
`include_tree` currently re-implements:
- **File Parsing**: Uses its own (slightly flawed) regex for `#include`.
- **Path Resolution**: Manually calculates paths instead of using the project's hierarchy knowledge.
- **Disk I/O**: Re-reads file contents already loaded by `project_core`.
- **Dependency Logic**: Re-calculates closures for redundancy detection.

## 2. Proposed Architecture: `project_tree`

The `project_tree` class will act as the single source of truth for the project's structure.

### 2.1. Unified Regex
A single, robust regex will be used for all tools:
```python
#   Captures: full string, opening delimiter, clean path, closing delimiter
re.compile(r'#include\s*(?P<full>(?P<open>[<"])(?P<path>[^>"]+)(?P<close>[>"]))')
```

### 2.2. Generic Data Structure

#### `project_tree` (The Container)
- `nodes`: Dictionary mapping absolute paths or system names to `tree_node` objects.
- `hierarchy_map`: Dictionary mapping project hierarchy strings (e.g., `sak/geometry/point`) to `tree_node` objects.
- `config`: Essential path information (`include`, `source`, `tests`).

#### `tree_node` (The Unit)
Represents any file involved in the project (internal or external).
- `path`: Full path (or None for system headers).
- `hierarchy`: Canonical name within the project.
- `content`: Cached content of the file.
- `is_external`: Boolean flag for STL or external libraries.
- `direct_includes`: List of `include_reference` objects.
- `closure`: Set of all transitive dependencies (computed once).

#### `include_reference` (The Link)
- `raw_text`: The full `#include ...` line.
- `display_text`: The `<path>` or `"path"` part.
- `target_node`: Reference to the `tree_node` being included.
- `is_redundant`: Boolean, `true` if the `target_node` is already present in the closure of another include in the same file.

## 3. Key Responsibilities of `project_tree`

1.  **Project Crawling**: Efficiently scanning directories and identifying C++ files.
2.  **System/STL Discovery**: Identifying headers that are not part of the project but are required for the "include tree" visualization.
3.  **Transitive Closure Calculation**: Pre-calculating the full set of dependencies for every node to allow O(1) redundancy checks.
4.  **Redundancy Identification**: flagging includes that are technically unnecessary because they are already provided by another direct include.
5.  **Hierarchy Resolution**: Translating `#include "path.hpp"` or `#include <header>` into actual file system locations or canonical names.

## 4. Integration Strategy

- **`project_core`**: Will instantiate `project_tree` and use its `hierarchy_map` and `closure` data to decide which files need recompilation and to build the linker's object list.
- **`include_tree`**: Will receive an initialized `project_tree` and focus solely on the visual formatting (the "tree" output), consuming the pre-calculated `is_redundant` and `is_external` flags.
- **`code_verifier`**: Can use `project_tree` to check for missing includes or circular dependencies.

## 5. Bonus Features for `include_tree`

- **External Visibility**: By setting `is_external = true`, `include_tree` can now show `<vector>`, `<ranges>`, etc., in the tree, which was previously ignored by `project_core`.
- **Memory/IO Efficiency**: Files are read exactly once.
- **Improved Accuracy**: The unified regex and resolution logic eliminate the "ghost" includes or missing dependencies caused by different parsing rules.

## 6. Conclusion

Separating the "Mapping" (`project_tree`) from the "Action" (`project_core`) simplifies the codebase, improves performance, and provides a powerful API for any future tool that needs to understand the project structure.
