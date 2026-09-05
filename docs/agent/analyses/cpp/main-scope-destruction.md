# Main Scope Destruction and Resource Management in C++

## 1. The Context: "The Cherno" OpenGL Bug

In a well-known video series on OpenGL and C++ game engine development, the author ("The Cherno") encountered a bug where the application would hang or crash upon closing. This led to a common misunderstanding that automatic objects (stack-allocated variables) inside the `main` function are not destroyed when the program exits.

In reality, the bug was not a failure of C++ to destroy stack objects, but rather an issue of **destruction order**.

### The Bug Pattern

Consider the following typical structure of an OpenGL application:

```cpp
auto main( ) -> int
{
	//	1. initialize library and create context
	glfwInit( );
	auto* window = glfwCreateWindow( 640, 480, "OpenGL", nullptr, nullptr );
	glfwMakeContextCurrent( window );

	//	2. allocate gpu resources on the stack
	vertex_buffer buffer( data );

	//	3. main loop
	while( not glfwWindowShouldClose( window ) )
	{
		//	render...
	}

	//	4. terminate context
	glfwTerminate( );

	return	0;
}
```

When this program exits:
1. `glfwTerminate( )` is called, which destroys the OpenGL context and releases GPU driver resources.
2. The `main` function returns.
3. As `main` exits, the stack is unwound, and the destructor of `buffer` (`~vertex_buffer( )`) is called.
4. The destructor attempts to call OpenGL functions (e.g., `glDeleteBuffers`) to release the GPU buffer.
5. Because the OpenGL context has already been destroyed by `glfwTerminate( )`, the driver calls are invalid. This leads to undefined behavior, infinite loops in error-checking macros, or immediate crashes.

### The Solution: Explicit Scope Control

To fix this, the GPU resources must be destroyed **before** the context is terminated. This can be achieved by wrapping the resource lifecycle in an inner scope:

```cpp
auto main( ) -> int
{
	glfwInit( );
	auto* window = glfwCreateWindow( 640, 480, "OpenGL", nullptr, nullptr );
	glfwMakeContextCurrent( window );

	{
		vertex_buffer buffer( data );
		while( not glfwWindowShouldClose( window ) )
		{
			//	render...
		}
	} // buffer is destroyed here, while the context is still active

	glfwTerminate( );
	return	0;
}
```

---

## 2. The Misunderstanding of Double Braces `{{ }}` in `main`

To prevent this issue, some developers adopt a convention of wrapping the entire body of `main` in an extra set of braces:

```cpp
auto main( ) -> int
{{
	//	code...
	return	0;
}}
```

This was done under the assumption that wrapping the body in an inner scope forces destruction before the final exit. However, this pattern is semantically redundant and can introduce unreachable code bugs:

1. **Redundancy:** If the inner scope contains the `return` statement, returning from the inner scope still triggers the destruction of all automatic objects in all enclosing scopes (including the outer `main` scope) in reverse order of construction. The destruction happens at the exact same point in execution.
2. **Unreachable Code:** If there is cleanup code placed between the inner closing brace and the outer closing brace (e.g., `glfwTerminate( )`), but the `return` statement is placed *inside* the inner scope, the cleanup code will never be executed because the function returns early.

Therefore, the double-brace pattern is unnecessary. Standard single braces `{}` should be used for `main`, and inner scopes should only be introduced locally around specific resources that require destruction before subsequent cleanup statements.

---

## 3. The Real Danger: `std::exit( )` vs `return`

While returning from `main` guarantees the destruction of stack-allocated objects, calling `std::exit( )` (or `std::quick_exit( )`, `std::abort( )`) does not.

### Standard Return from `main`

When `main` terminates via a `return` statement (or by reaching the end of the function):
1. All automatic (stack) objects in `main` are destroyed.
2. Static and global objects are destroyed.
3. Functions registered with `std::atexit( )` are called.
4. The process terminates.

### Termination via `std::exit( )`

When `std::exit( )` is called from anywhere in the program:
1. **No automatic (stack) objects are destroyed.** The stack is not unwound.
2. Functions registered with `std::atexit( )` are called.
3. Static and global objects are destroyed.
4. The process terminates.

Any resource managed by a stack-allocated RAII object (such as file handles, database connections, or GPU memory) will be leaked at the application level if `std::exit( )` is called. While modern operating systems reclaim memory and file descriptors upon process termination, GPU drivers and external network resources may not clean up immediately, leading to resource exhaustion or locked states.

### Recommendation

1. **Prefer `return`:** Always exit the program by returning an exit code from `main`. Use exceptions or error propagation (e.g., `std::expected` or `std::optional`) to bubble errors up to `main` rather than calling `std::exit( )` in deeply nested code.
2. **Avoid `std::exit( )`:** Do not use `std::exit( )` in C++ applications unless absolutely necessary (e.g., in a child process immediately after a fork before exec, or when terminating due to an unrecoverable system panic where stack unwinding is impossible).
3. **Use RAII and Standard Scopes:** Rely on standard single-brace scopes `{}` for `main`. If a resource must be destroyed before a specific cleanup function is called, wrap only that resource and its usage in a localized inner scope.
