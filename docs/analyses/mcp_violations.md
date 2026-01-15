# MCP Structure Analysis - Violations Report

This document lists violations of the Model Context Protocol (MCP) architectural rules regarding path management.

## 1. Absolute Paths Violations
**Rule:** Absolute paths are forbidden; only relative paths must be used.

### `tools/agent_statistic.py`
- **Line 32:** `_statistic_file = "/home/.gemini/statistic.json"`
  - **Violation:** Uses a hardcoded absolute path to a system directory.

### `tools/include_tree.py`
- **Line 35:** `abs_path = os.path.abspath( target_file )`
  - **Violation:** Converts input to an absolute path for internal processing.
- **Line 79:** `output.append( root_node.path if root_node.path else root_node.hierarchy )`
  - **Violation:** Prints the absolute path stored in the node if it exists.

### `tools/lib/project_tree.py`
- **Line 74:** `abs_path = os.path.abspath( path )`
  - **Violation:** Generates absolute paths during filesystem scanning.
- **Line 77:** `self.nodes[ abs_path ] = node`
  - **Violation:** Uses absolute paths as primary keys in the project tree map.
- **Line 104:** `self.nodes[ include_path ] = node`
  - **Violation:** Stores paths that may be absolute or external directly in the nodes map.

---

## 2. Hardcoded Paths Violations
**Rule:** Hardcoded folder names and paths are forbidden; all paths must be retrieved from the `project_core` configuration.

### `tools/project_mcp.py`
- **Line 21:** `process = _invoke_subprocess( [ "python3", f"tools/{name}.py", ... ] )`
  - **Violation:** Folder name `"tools/"` is hardcoded.

### `tools/file_generator.py`
- **Line 45:** `file_path_hpp = f"include/{ '/'.join( hierarchy_list ) }.hpp"`
  - **Violation:** Folder name `"include/"` is hardcoded.
- **Line 59:** `file_path_cpp = f"source/{ '/'.join( hierarchy_list ) }.cpp"`
  - **Violation:** Folder name `"source/"` is hardcoded.
- **Line 77:** `adhoc_dir = "tests/adhoc"`
  - **Violation:** Folder name `"tests/adhoc"` is hardcoded.
- **Line 99:** `file_path = f"tests/{path}/{filename}"`
  - **Violation:** Folder name `"tests/"` is hardcoded.
- **Line 101:** `file_path = f"tests/{filename}"`
  - **Violation:** Folder name `"tests/"` is hardcoded.

### `tools/lib/template_engine.py`
- **Line 27:** `def __init__( self, name, path = "docs/templates" ):`
  - **Violation:** Default path `"docs/templates"` is hardcoded.

### `tools/lib/metadata_provider.py`
- **Line 35:** `prefixes = [ "include/", "source/", "tests/" ]`
  - **Violation:** Directory prefixes are hardcoded instead of being read from `project_core` config.

### `tools/agent_statistic.py`
- **Line 32:** `_statistic_file = "/home/.gemini/statistic.json"`
  - **Violation:** The entire file path is hardcoded.

### `tools/lib/project_core.py`
- **Architectural Omission:** The `DEFAULT_CONFIG["paths"]` lacks definitions for `tools`, `docs`, and `templates` directories, forcing other tools to hardcode these values.
