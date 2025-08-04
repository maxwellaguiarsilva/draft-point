# Workflow for Code Changes

This document outlines the steps for pushing code changes to GitHub.

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

3.  **Create a New Branch:**
    Create a new branch for your changes. Replace `<branch-name>` with a descriptive name for your feature or fix.
    You must choose the branch name according to the information obtained in git diff.
    ```bash
    git checkout -b <branch-name>
    ```

4.  **Commit Changes:**
    Commit your changes, preferably individually for better clarity.
    You must choose the commit message according to the information obtained in git diff.
    ```bash
    git add .
    git commit -m "commit message"
    ```

5.  **Pull Request with `gh` CLI:**
    Use the `gh` (GitHub CLI) tool to automate the pull request process.
    You must choose the pull request title and body according to the summary of descriptions of commits made.
    *   **Push Branch:**
        Push your new branch to the remote repository.
        ```bash
        git push origin <branch-name>
        ```

    *   **Open a Pull Request:**
        This command will open a pull request for your current branch, using commit messages for the title and body.
        ```bash
        gh pr create --fill-verbose
        ```

    *   **Merge Pull Request:**
        Once the pull request is ready to be merged, use the following command to approve and merge it into the main branch. This will also delete the feature branch from both the local and remote repositories.
        ```bash
        gh pr merge --auto --delete-branch --merge
        ```

6.  **Clean Up Stale Remote Branches:**
    Remove local references to remote branches that no longer exist on the remote repository.
    ```bash
    git remote prune origin
    ```

7.  **Final Validation:**
    Perform a final check to ensure everything is in order after the merge and cleanup.
    ```bash
    git status
    git diff
    git branch -a
    ```

