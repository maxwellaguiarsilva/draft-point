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
| `target_node` | `target` | The destination file of an include directive. |
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
  - `direct_includes`: List of immediate `include_reference` objects.
  - `dependencies`: Set of all transitive `project_file` dependencies.

- **`cpp` (Specialization)**: A source file with specific compilation needs.
  - `is_main`: Boolean identified via regex in content (pre-computed knowledge).
  - `is_test`: Boolean identified via path (e.g., starts with `tests/`).
  - `object_path`: Computed path for the `.o` file in the build directory.

- **`hpp` (Specialization)**: A header file.
  - Note: External dependencies (like `<vector>` or `<string>`) are instantiated as `hpp` objects with `is_external = True`.

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
