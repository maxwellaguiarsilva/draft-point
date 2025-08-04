# Quick Upload Workflow

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

5.  **Merge Locally and Push:**
    Merge your feature branch into the main branch locally, and then push the changes to the remote repository.
    ```bash
    git checkout main
    git merge <branch-name>
    git push origin main
    git branch -d <branch-name>
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

