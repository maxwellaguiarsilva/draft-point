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
    Analyze the modifications to determine if they cover distinct topics. Unrelated changes must be committed separately for better clarity and history tracking. **It is strictly forbidden to commit unrelated changes in the same commit.**
    You must choose the commit message according to the information obtained in git diff.
    The agent should not ask for approval for the commit message and should proceed with the commit automatically.
    **Imperative:** If your commit message contains backticks (``` ` ```), you MUST escape them with a backslash (```\```) to avoid command substitution errors.
    ```bash
    # For each distinct group of changes:
    git add <specific_files_or_directories>
    git commit -m "commit message describing only these changes"
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

6.  **Communication:**
    Inform the user that the procedure was completed successfully.
    In this final communication, show exactly which commit message was used, enclosed in `backticks`.

7.  **Register Success:**
    Upon successful completion of the upload, the agent must call the `agent_statistic` tool with the `name` parameter set to `"success"`.

