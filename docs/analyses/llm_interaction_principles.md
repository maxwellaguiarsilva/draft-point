# LLM Interaction Principles

This document outlines the core principles regarding interaction with Large Language Models (LLMs) in this project, focusing on avoiding common pitfalls and maintaining high architectural standards.

## Common Frustrations and Pitfalls

Experience has shown that the primary frustrations when working with LLMs often stem from:

1.  **Regression of functionality:** Introducing bugs or breaking existing features during refactoring.
2.  **Violation re-introduction:** The refactoring itself introducing the very violations (like SRP or DRY) it was intended to resolve.
3.  **Lack of refinement:** Prioritizing immediate implementation over planning.
4.  **Skipping small steps:** Attempting large, complex changes in a single leap.
5.  **Context window limitations:** Overestimating the LLM's ability to solve complex problems in a single session.

## Core Interaction Philosophy

To mitigate these issues, we adhere to the following hierarchy of priorities:

1.  **Honesty and Depth Recognition:** Above all, be honest about the complexity and depth of the task. Do not underestimate "small" problems. Real depth must be confronted and analyzed thoroughly. A good agent should make the user confront the real depth of the problem.
2.  **Iterative Small Steps:** Focus on solving one small, verifiable problem at a time per chat session. No complex task is solved in a single leap. Iterative refinement ensures each step is correct before proceeding. Record the current status in the documents and ask the user to clear the session after finishing the current process.
3.  **Principles over Delivery:** Technical principles (SRP, DRY, Visual Serenity) are non-negotiable and must be maintained throughout the process. We never prioritize "finishing the task" over "doing it right". A small and partial delivery is better than a complete one without adhering to these principles.

### Operational Rules

-   **No unilateral decisions:** If a request is ambiguous or problematic, it must be discussed and clarified first.
-   **Tool-first verification:** Never attempt to verify project-specific rules (like formatting) manually if a tool exists for it.

## Anti-Patterns to Avoid

-   **Direct MCP calls via shell:** Never attempt to invoke an MCP tool script directly using `python3` or via shell commands if it is registered as an MCP tool. Use the MCP mechanism.
-   **Manual formatting verification:** Do not trust your own eyes for project-specific formatting rules. Always use `cpp_code_verifier` or `python_code_verifier`.
-   **Ignoring established libraries:** If a utility exists in `sak/` or `tools/lib/`, use it instead of reimplementing logic.
