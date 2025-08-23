#   Bash Coding Style Guidelines

This document outlines the preferred Bash coding style. Adhering to these guidelines ensures consistency and readability across Bash scripts in this project.

##  1. Indentation
*   Use **tabs** for all indentation. Do not use spaces for indentation.

##  2. Comments and literal strings
*   Start comments with `#\t` (the tab character, not the slash and "t" ).
*   All comments must be full lower case and the sentences do not have a full stop.
*   Instead of using a period to start a new sentence, put the other sentence on a new line. One sentence per line.
*   Literal strings must be in full lower case and sentences must not have a period at the end.

##  3. Variable Naming
*   Use `snake_case` (lowercase letters with underscores) for all variable names (e.g., `my_variable`, `file_extension`).
*   Declare associative arrays using `declare -A` (e.g., `declare -A project`).
*   Declare indexed arrays using `declare -a` (e.g., `declare -a lib_list`).

##  4. Function Definitions
*   Define functions using the `function_name( )
{
    ...
}` syntax.
*   Use the `local` keyword for all variables declared within a function to ensure they are scoped to that function.

##  5. Variable Expansion
*   Always use double quotes and curly braces for variable expansions to prevent word splitting and globbing, and to ensure clarity.
    *   **Good:** `foo="text-${bar}"`
    *   **Bad:** `foo=text-$bar`

##  6. Conditional and Loop Statements
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
##  7. Parentheses and spaces
*   Parentheses have a space after opening and before closing.
*   Use spaces in expressions
    *   **Good:**
        ```bash
        my_function_name( )
        {
            echo "$( ls )"
            local -i num_a num_b
            num_a=4
            num_b=5
            echo "$(( num_a * num_b ))"
        }
        ```
    *   **Bad:**
        ```bash
        my_function_name()
        {
            echo "$(ls)"
            local -i num_a num_b
            num_a=4
            num_b=5
            echo "$((num_a*num_b))"
        }
        ```
##  8. Script Structure
*   Organize scripts with global variable declarations at the top.
*   Follow global variables with all function definitions.
*   Place the main script logic (calls to functions, etc.) at the bottom of the file.

##  9. Validation.
*   After producing any code in Bashscript, perform a final validation to ensure each of the previous items has been strictly followed.
    *   If any of them are not in compliance with this document, use the available tools to make the necessary adjustments.
*   Do not consider the code to have been properly delivered until this validation is 100% okay.

##  10. Validation with shellcheck.
*   After completing the previous validation, perform a new validation using the "shellcheck" tool.
    *   If any part of the code is not in compliance with this document, use the available tools to make the necessary adjustments.


