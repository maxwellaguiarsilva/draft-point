# **THIS IS A C++26 PROJECT**

There is a mindset threshold for anyone working on this project, and it is defined in the `point.hpp` class. This project was created for the sole purpose of preventing code like the following:
```C++
//  very bad
std::array<float, 3> cross( const float left[3], const float right[3] )
{
    return {
         left[1] * right[2] - left[2] * right[1]
        ,left[2] * right[0] - left[0] * right[2]
        ,left[0] * right[1] - left[1] * right[0]
    };
}
//  very good
const point cross( const point left, const point right ) { return rotated( left, 1 ) * rotated( right, 2 ) - rotated( left, 2 ) * rotated( right, 1 ); }
```
If the code opens up symmetric operations by dimension, then it renders this project useless. This project only exists for that reason. If the code works, but it didn't abstract that symmetry, any other existing solution that does it that way could be used, so there's no added value to the project in solutions of that type.

Whenever you discuss or address any of the issues below, please attach the relevant files first. Having this context is mandatory before discussing these topics:
- C++: `read-file docs/agent/style-guide/cpp.md using.hpp sak.hpp point.hpp`.
- project-mcp-tools: `read-file ../project-mcp-tools/README.md`

This project is an educational exploration with the following guidelines:
- Project tools are invoked through the MCP port. The wrappers in `./scripts` and the CLI are reserved for human use.
- `./scripts` and `./dist` are **ALREADY** on `PATH`: their commands are invoked from the current working directory without the directory prefix.
- Extreme Don't Repeat Yourself: abstraction is chosen even when usage and the trade-off in economy is low ( repetitions >2 ).
- Includes transitive: call the mcp tool `cpp_include_tree` with `flg_auto_fix`. Before any compile, call that tool regardless of the impact its changes may have on the files.
- Never explicitly and redundantly redeclare what is already happening through transitivity of the consuming element. Example: a class that has unique_ptr does not need to explicitly delete the copy constructor.

- When creating a temporary/ephemeral C++ test unit, prefer using mcp tool `cpp_create_test` with the adhoc flag + `cpp_compile`, instead of `/tmp/ + manual command g++`.

