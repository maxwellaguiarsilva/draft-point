# **THIS IS A C++26 PROJECT**

Whenever you discuss or address any of the issues below, please attach the relevant files first. Having this context is mandatory before discussing these topics:
- C++: `read-file style-guide/cpp.md using.hpp sak.hpp`.
- project-mcp-tools: `read-file ../project-mcp-tools/README.md`

This project is an educational exploration with the following guidelines:
- Extreme Don't Repeat Yourself: abstraction is chosen even when usage and the trade-off in economy is low ( repetitions >2 ).
- Includes transitive: execute the command `cpp-include-tree --flg-auto-fix` in the terminal.
- Never explicitly and redundantly redeclare what is already happening through transitivity of the consuming element. Example: a class that has unique_ptr does not need to explicitly delete the copy constructor.

