# LLM Interaction Principles


This document outlines the core principles and observations regarding interaction with Large Language Models (LLMs) in this project, focusing on avoiding common pitfalls and maintaining high architectural standards.


## Common Frustrations and Pitfalls

Experience has shown that the primary frustrations when working with LLMs often stem from:

1.  **Regression of functionality:** Introducing bugs or breaking existing features during refactoring processes.
2.  **Violation re-introduction:** The refactoring itself introducing the very violations (like SRP or DRY) it was intended to resolve.
3.  **Lack of refinement:** Prioritizing immediate implementation over the necessary refinement and planning before putting it into practice.
4.  **Skipping small steps:** Attempting large, complex changes in a single leap rather than taking small, certain steps in the right direction.
5.  **Context window limitations:** Overestimating the LLM's ability to solve real problems in a single session, especially when limited by the context window.


## Core Interaction Philosophy

To mitigate these issues, we adhere to the following principles:

-   **No unilateral decisions:** The LLM must not make significant decisions on its own. If a request is ambiguous or problematic, it must be discussed and clarified first.
-   **Principles over delivery:** We never prioritize delivery over established project principles (SRP, DRY, etc.). Frustrations arise when the LLM prioritizes "finishing the task" over "doing it right".
-   **Depth recognition:** We avoid underestimating seemingly "small" problems. Real depth must be confronted and analyzed thoroughly. A good agent should make the user confront the real depth of the problem.
-   **Iterative refinement:** We work in small, verifiable increments, ensuring each step is correct before proceeding. No real problem is solved in a single chat session.