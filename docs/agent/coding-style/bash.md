# Bash Coding Style Guidelines

This document outlines the preferred Bash coding style. Adhering to these guidelines ensures consistency and readability across Bash scripts in this project.

## 1. Indentation
*   Use **tabs** for all indentation. Do not use spaces for indentation.

## 2. Comments
*   Start comments with a single hash (`#`).

## 3. Variable Naming
*   Use `snake_case` (lowercase letters with underscores) for all variable names (e.g., `my_variable`, `file_extension`).
*   Declare associative arrays using `declare -A` (e.g., `declare -A project`).
*   Declare indexed arrays using `declare -a` (e.g., `declare -a lib_list`).

## 4. Function Definitions
*   Define functions using the `function_name( )
{
    ...
}` syntax.
*   Use the `local` keyword for all variables declared within a function to ensure they are scoped to that function.

## 5. Variable Expansion
*   Always use double quotes and curly braces for variable expansions to prevent word splitting and globbing, and to ensure clarity.
    *   **Good:** `foo="text-${bar}"`
    *   **Bad:** `foo=text-$bar`

## 6. Conditional and Loop Statements
*   Use `[[ ... ]]` for conditional expressions within `if`, `while`, and `for` statements.
*   Place `then` (for `if`) or `do` (for `for` and `while`) on a new line after the condition.
    *   **Good:**
        ```bash
        if [[ -f "${file_path}" ]];
        then
            echo "file exists"
        fi

        for item in "${some_list[@]}";
        do
            echo "item: ${item}"
        done

        while read -r line;
        do
            echo "line: ${line}"
        done <<< "${my_text}"
        ```
    *   **Bad:**
        ```bash
        if [ -f "file.txt" ]; then
            echo "File exists."
        fi

        for i in "${my_array[@]}"; do
            echo "Item: $i"
        done

        while read -r line; do
            echo "Line: $line"
        done <<< "$my_text"
        ```

## 7. Script Structure
*   Organize scripts with global variable declarations at the top.
*   Follow global variables with all function definitions.
*   Place the main script logic (calls to functions, etc.) at the bottom of the file.

