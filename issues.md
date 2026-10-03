# Issues

Pre-existing style violations discovered during the shader refactor. Both are currently unfixed.

## using enum spaced instead of tab-separated

- reference: `source/sdl3_demo.cpp:306`

```cpp
using enum window::flag;
```

violates the project rule that the tokens of a `using enum` directive are separated by tabs (cf. the tab separation used in `include/sak/opengl/shader.hpp`), not spaces.

## scope-qualified exchange at call site

- reference: `source/sdl3_demo.cpp:183`

```cpp
auto consume_changed( ) noexcept -> bool { return ::std::exchange( m_changed, false ); }
```

violates the project `using` rule that avoids the scope operator at usage sites because `exchange` is never imported.

## scope note

these two violations were intentionally left out of scope for the shader refactor per the maintainer's decision.
