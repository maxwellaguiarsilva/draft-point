# Simple Upload Workflow

This document outlines the steps for quickly pushing code changes to GitHub without a pull request.

1.  **Fetch Latest Code:**
    Execute `git pull` to retrieve the most recent code from the remote repository.
    ```bash
    git pull
    ```

2.  **Analyze Changes:**
    Review the `git diff` to understand what modifications have been made in the project's code.
    ```bash
    git status
    git diff
    ```

3.  **Commit Changes:**
    Commit your changes, preferably individually for better clarity.
    You must choose the commit message according to the information obtained in git diff.
    The agent should not ask for approval for the commit message and should proceed with the commit automatically.
    **Warning:** If your commit message contains backticks (``` ` ```), you must escape them with a backslash (```\```) to avoid command substitution errors.
    ```bash
    git add .
    git commit -m "commit message with \`backticks\` on text"
    ```

4.  **Push:**
    Push the changes to the remote repository.
    ```bash
    git push
    ```

5.  **Final Validation:**
    Perform a final check to ensure everything is in order.
    ```bash
    git status
    git diff
    ```

