# Project Check Workflow

This document outlines the procedure for performing a comprehensive evaluation of the project to ensure its integrity and quality.

## Procedure

1.  **Do Not Modify Files:**
    Under no circumstances should any files be modified during this check.

2.  **Focus on Functional Integrity:**
    The evaluation must prioritize finding critical errors, logic flaws, memory safety issues, and undefined behaviors. **Do not** report on code style compliance or aesthetic preferences, as these are explicitly out of scope for this workflow.

3.  **Identify Significant Issues:**
    Flag issues that, while not critical bugs, represent technical debt, potential maintenance problems, or confusing logic that should be signaled to the developer.

4.  **Respect Intentional Issues:**
    Some errors or bugs may be accompanied by comments in the code explicitly stating they should be ignored. In such cases, they must be disregarded.

5.  **Ignore Unused Assets:**
    Do not include unused classes or files in the evaluation.

6.  **Starting Point:**
    Begin the evaluation process from the `source/main.cpp` file.

## Customization

When asking to execute this workflow, you can specify the language in which you wish to receive the response.
