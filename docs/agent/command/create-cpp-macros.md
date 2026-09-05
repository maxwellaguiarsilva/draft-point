---
description: Refactor symmetric methods or functions in a C++ header using local generator macros adhering to project conventions.
---

You are requested to analyze and refactor symmetric methods, overloads, or functions in the specified C++ header file using local generator macros.

## Conventions for C++ Macros in this Project

1. **Header Guard Numeric ID Prefix:**
   - Every header defines a numeric header guard: `#ifndef header_guard_<numeric_id>`.
   - Local macros inside that file MUST be prefixed with `__<numeric_id>_`.
2. **Local Scope & Cleanup:**
   - Define macros right above their usage site or right above the class definition.
   - Always `#undef` every defined macro at the end of the namespace / file.
3. **Macro Usage & Formatting:**
   - Multi-line macros use `\` continuation at the end of lines.
   - Comma-delimited arguments to macros must respect spacing rules: `( a_name, a_param1, a_param2 )`.
   - Macros that instantiate multiple members or overloads should eliminate boilerplate and repeated patterns while preserving exact semantics, types, and const/noexcept correctness.
   - When applicable, variadic macros or helper macros like `__use_macro` (from `sak/using.hpp`) can be used to invoke definitions over list groups.
4. **Validation Pipeline:**
   - Run `cpp_code_verifier` on the modified file.
   - Run `cpp_compile` to guarantee no compiler errors.
   - Run the relevant unit tests (`./dist/test-...`).

## Procedure

1. **Locate Target File:**
   - Extract the file path from `$ARGUMENTS`. If not an absolute or directly resolvable relative path, find it using `find -L . -name "*<file_name>*"`.
2. **Read and Analyze:**
   - Read the file using the `read` tool.
   - Identify the numeric header guard ID (`header_guard_<id>`).
   - Identify repeated, symmetric method patterns.
3. **Draft and Apply Generator Macros:**
   - Create focused macro templates with the `__<id>_` prefix.
   - Replace the boilerplate implementations with invocations of the generator macro.
   - Add `#undef` directives corresponding to each macro created.
   - Apply edits via the `edit` tool.
4. **Verify Standards and Compilation:**
   - Verify formatting with `project-mcp-tools_cpp_code_verifier`.
   - Recompile using `project-mcp-tools_cpp_compile`.
   - Execute the corresponding test binary to ensure no regressions.
5. **Report:**
   - Report the outcome concisely: the file modified, the macros introduced, and test results.

User text: `$ARGUMENTS`
