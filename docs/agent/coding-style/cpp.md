# Coding Style Guidelines for C++ Project

This document outlines the coding style conventions to be followed for this project. Adhering to these guidelines ensures consistency and readability across the codebase.

## General Rules

*   **No Comments in Code Implementation:** Code should be self-documenting. Avoid adding comments within the code's implementation logic. License headers and file description comments at the top of a file are permitted.
*   **Naming Convention:** All identifiers (variables, functions, classes, namespaces, etc.) must use `snake_case`. `CamelCase`, `PascalCase`, `kebab-case`, or any other casing style is forbidden.
    *   **Example (Good):**
        ```cpp
        int my_variable;
        void process_data( );
        class my_class;
        ```
    *   **Example (Bad):**
        ```cpp
        int myVariable;
        void processData();
        class MyClass;
        ```
*   **Parentheses and Brackets Spacing:** Always include a single space immediately after an opening parenthesis `(` or square bracket `[` and immediately before a closing parenthesis `)` or square bracket `]`.
    *   **Example (Good):**
        ```cpp
        if( condition )
            do_something( );
        my_array[ index ];
        ```
    *   **Example (Bad):**
        ```cpp
        if(condition)
            doSomething();
        myArray[index];
        ```
*   **Single-Line Control Structures:** For `if`, `for`, and `while` statements that contain only a single line of code, omit the curly braces `{}`. Additionally, there should be no space between the keyword and the opening parenthesis.
    *   **Example (Good):**
        ```cpp
        if( condition )
            do_something( );
        for( init; condition; ++count )
            do_something_else();
        while( condition )
            another_thing( );
        ```
    *   **Example (Bad):**
        ```cpp
        if (condition) {
            doSomething();
        }
        ```
*   **Increment/Decrement Operators:** Always prefer pre-increment (`++count`) over post-increment (`count++`) and pre-decrement (`--count`) over post-decrement (`--count`).
    *   **Example (Good):**
        ```cpp
        ++count;
        ```
    *   **Example (Bad):**
        ```cpp
        count++;
        ```
*   **`using` Clauses:**
    *   Use `using` clauses to avoid fully qualifying type names.
    *   **Example:**
        ```cpp
        using ::std::chrono::high_resolution_clock;
        // ...
        auto start_time = high_resolution_clock::now( );
        ```
    *   **Scope:** In header files (`.hpp`), `using` clauses must always be placed within the smallest possible scope (e.g., inside a function, class, or namespace), never at the global file scope. In implementation files (`.cpp`), `using` clauses may be placed at the global file scope.
