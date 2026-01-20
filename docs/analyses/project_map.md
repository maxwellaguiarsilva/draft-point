# Project Map: Static Architecture and Semantic Elevation

## 1. Vision and Objective
The goal of this refactoring is to decouple the **static knowledge** of the project (file structure, types, and dependencies) from its **dynamic execution** (compilation, linking, and analysis). By elevating the semantics, we bring the codebase closer to the domain of C++ engineering, removing generic computer science abstractions in favor of specialized entities.

## 2. Semantic Elevation (Terminology)
To reduce visual noise and improve intent, the following terms are redefined:

| Current (CS Jargon) | Proposed (Domain Term) | Context |
| :--- | :--- | :--- |
| `tree_node` | `project_file` | The base entity representing any file in the project. |
| `nodes` | `files` | The collection of all tracked entities. |
| `closure` | `dependencies` | The transitive set of files required by a source. |
| `project_tree` | `project_map` | The component that tracks the static state of the project. |

## 3. The Static Layer (`project_map`)
This layer is responsible for discovery, classification, and dependency resolution. Entities here are pure data carriers and do not know about the "runtime" (compilers or build state).

### 3.1. Entity Hierarchy
- **`project_file` (Base)**: Represents a generic file.
  - `path`: Path from the project root (None for external files).
  - `hierarchy`: Identification path or include string for external files (e.g., `game/player`).
  - `is_external`: Boolean indicating if the file is outside the project (e.g., system headers).
  - `modified_at`: Last modification timestamp (min value for external files).
  - `content`: Lazy-loaded file content (None for external files).
  - `includes`: Set of immediate `project_file` objects.
  - `dependencies`: Set of all transitive `project_file` dependencies.

- **`hpp` (Specialization)**: A header file.
  - Note: External dependencies (like `<vector>` or `<string>`) are instantiated as `hpp` objects with `is_external = True`.

- **`cpp` (Specialization)**: A source file with specific compilation needs.
  - `is_main`: Boolean identified via regex in content (pre-computed knowledge).
  - `is_test`: Boolean identified via path (e.g., starts with `tests/`).
  - `object_path`: Computed path for the `.o` file in the build directory.

### 3.2. Data Intelligence
Properties like `is_main`, `is_test`, and `object_path` are computed during discovery within `project_map`. This removes the need for the runtime layer to "investigate" files, allowing it to act on pre-computed knowledge.
For `hpp` objects where `is_external` is True, the data (content, includes) is not populated, and consumers of the graph must respect this contract by checking the flag before accessing such fields.

## 4. Path Convention
To maintain portability and simplicity, all paths used within the system follow these rules:
- **Project-Centric**: All paths are stored as paths starting from the project root. Full filesystem paths (starting from the system root `/`) are strictly forbidden.
- **Naming Purity**: The terms `absolute` and `relative` are forbidden in variable and attribute names. Since all paths are project-centric by design, the nature of the path is an inherent property of the system, not a qualifier.

## 5. The Runtime Layer (`project_core`)
The core consumes the `project_map` to perform actions. It manages the "machinery" of the project.

- **Responsibilities**:
  - Orchestrating parallel compilation of `cpp` entities.
  - Linking object files via `binary_builder`.
  - Managing thread-safe output and global build state (`stop`, `is_stopped`).
  - Interfacing with external tools (`clang`, `cppcheck`).

- **Decoupling**:
Entities in the map no longer hold a reference to the `project` object. Runtime-only state, such as `compiled_at`, is handled by the core or builders that operate on the map's data, ensuring a "Single Source of Truth" for the project structure.

## 5. Technical Implementation Details
- **Ambiguity Resolution**: `project_map` maintains a `hierarchy_items` map (formerly managed by `project_core`) to correctly group `.cpp` and `.hpp` files that share the same hierarchy.
- **Dependency Inversion**: Instead of files asking the project for the compiler, the project (or a specialized builder) passes the necessary data to the compiler to act on the files.
- **Transitive Integrity**: The `dependencies` (transitive closure) calculation remains in the static layer, as it represents the structural relationship between files, regardless of whether they are being compiled or not.

## 6. Refactoring Log (State Reference)
**This section is the primary reference for the current state of the refactoring. If context is lost, start here.**

### 6.1. Progress (What has been done)

- **File Renaming**: `tools/cpp/cpp_lib/project_tree.py` was renamed to `tools/cpp/cpp_lib/project_map.py`.

- **Semantic Elevation**:

  - `tree_node` renamed to `project_file`.

  - `nodes` renamed to `files`.

  - `closure` renamed to `dependencies`.

  - `project_tree` renamed to `project_map`.

- **Static Layer Implementation**: 

  - `hpp` and `cpp` specializations were moved from `project_core` to `project_map`.

  - Logic for `is_main`, `is_test`, and `object_path` was moved to the `cpp` entity in `project_map`.

  - `project_map` now handles `hierarchy_items` and `path_map`.

- **Runtime Layer Refactoring**:

  - `project_core.py` was refactored to remove redundant entity definitions.

  - `build()` logic moved from `cpp` entity to `project_core.build_file()`.

  - `binary_builder` updated to consume `project_map` data.

- **Tool Integration**:

  - `compile.py`, `analyze.py`, `create_class.py`, and `create_test.py` were updated to the new architecture.

- **Verification**:

  - Both `analyze` and `compile` were executed successfully, confirming the stability of the refactored system.



### 6.2. Decisions Made

- **JIT Time-stamping**: `dependencies_modified_at` is a `@property` in `project_file` that calculates the maximum `modified_at` of the file and its dependencies on the fly. This ensures data freshness without manual updates.

- **Dependency Deletion**: Entities in `project_map` MUST NOT hold references to the `project` or `project_core` objects. They are pure data carriers.

- **Path Purity**: All paths are project-centric (no leading `/`). The terms `absolute` and `relative` are forbidden in names.

- **Entity State**: `update_compiled_at()` and `compiled_at` remain in the `cpp` entity within `project_map.py`, as they represent file-state knowledge.

- **Time Comparison**: Use `get_modification_time` consistently to return `datetime` objects for comparison, avoiding mixing with `float` from `os.path.getmtime`.



### 6.3. Next Steps

- **Refactoring Complete**: No immediate next steps required for this task. The static and runtime layers are now decoupled and semantically elevated.


