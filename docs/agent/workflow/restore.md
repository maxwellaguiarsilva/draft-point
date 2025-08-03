# Restoring Files or Directories

This document provides instructions on how to correctly restore a file or directory to its state in the last commit, handling cases where changes might be staged or unstaged.

## Procedure

1.  **Check Current Status:**
    Before attempting to restore, always check the current status of the repository and the specific file/directory using `git status`. This helps in understanding if the changes are staged, unstaged, or both.
    ```bash
    git status
    ```

2.  **Restore File/Directory:**
    To ensure a complete restoration to the state of the last commit (HEAD), a two-step process is required if the file/directory has changes that are both staged and unstaged.

    *   **Step 1: Unstage Changes (if any)**
        If `git status` indicates that the file or directory has "Changes to be committed" (i.e., it's staged), first unstage these changes.
        ```bash
        git restore --staged <path/to/file_or_directory>
        ```

    *   **Step 2: Discard Working Directory Changes**
        After unstaging (or if there were no staged changes to begin with), discard any remaining modifications in the working directory.
        ```bash
        git restore <path/to/file_or_directory>
        ```

    Replace `<path/to/file_or_directory>` with the actual path to the file or directory you wish to restore.

3.  **Verify Restoration:**
    After executing the restore commands, run `git status` again to confirm that the file or directory is no longer listed as modified and the working tree is clean.
    ```bash
    git status
    ```