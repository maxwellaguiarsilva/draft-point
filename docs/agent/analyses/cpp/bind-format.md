# `bind_front` with `std::format` in C++26

Technical analysis of combining `std::bind_front` with `std::format`, explaining the root causes of compiler rejections and presenting the validated idiomatic solution.

---

## 1. Problem Statement

In code generation pipelines and formatting helpers (such as terminal ANSI escape sequence generators), it is natural to attempt partially applying `std::format` via `std::bind_front`:

```cpp
// naive attempt: expected to fix format string and color code, leaving index free
using fmt_ptr = string (*)( std::format_string< int, int >, int&&, int&& );
auto ptr = static_cast< fmt_ptr >( format< int, int > );
println( "{}", bind_front( ptr, "\033[38;5;{}m", 0 )( 12 ) );
```

This fails to compile with errors resembling:
`call to consteval function 'std::_Binder<...>::operator()' is not a constant expression`.

Attempting to pass `std::string` instead of `const char*` results in the exact same failure.

---

## 2. Root Causes Analysis

Three distinct mechanisms in the C++ standard interact to prevent naive binding:

### 2.1 Overload Ambiguity
`std::format` has two overloaded function templates in header `<format>`:
```cpp
template< class... Args > string  format( format_string< Args... > fmt, Args&&... args );
template< class... Args > wstring format( wformat_string< Args... > fmt, Args&&... args );
```
Both overloads share the identical template parameter list `< class... Args >`. Specifying template arguments such as `std::format< int, int >` does not disambiguate the overload set because both `char` and `wchar_t` specializations are equally viable. An explicit cast to a concrete function pointer (or a lambda wrapper) is strictly required to pick one overload.

### 2.2 `consteval` Constructor and Immediate Escalation (P2564)
`std::format` takes `std::format_string< Args... >`, an alias for `std::basic_format_string< char, type_identity_t< Args >... >`. Its converting constructor:
```cpp
template< class T >
consteval basic_format_string( const T& s );
```
is **`consteval`** to enforce compile-time format string validation.

When a string literal (`const char[N]`) or a `std::string` is passed to `std::bind_front`:
1. `std::bind_front` decays and stores the argument as an internal member variable (`const char*` or `std::string`).
2. When the returned binder's `operator()` is invoked, it attempts to pass this stored member to the function pointer expecting `std::format_string`.
3. Converting that member invokes the `consteval` constructor of `basic_format_string`.
4. Because member variables of runtime objects are not constant expressions, this conversion is invalid at runtime.
5. Under C++23 / C++26 Immediate Escalation rules (P2564), because `_Binder::operator()` contains an immediate-escalating expression, the call operator itself is promoted to an immediate function (`consteval`).
6. Calling `binder( 12 )` in runtime context is therefore rejected as a call to a `consteval` function that is not a constant expression.

Passing `std::string` fails for the exact same reason: `basic_format_string`'s converting constructor is `consteval` and cannot be called with a runtime `std::string` instance.

### 2.3 Forwarding References and Value Categories (`Args&&` vs `int&&`)
`std::format` is declared as:
```cpp
template< class... Args >
string format( format_string< Args... > fmt, Args&&... args );
```
When instantiated with value types like `format< int, int >`:
- `Args&&...` becomes `int&&, int&&` (rvalue references).
- The stored bound argument `0` is an `int` held inside the binder object.
- When `bound( 12 )` is called on an lvalue binder, the binder forwards its stored member as an lvalue reference: `int&`.
- An lvalue reference `int&` cannot bind to an rvalue reference `int&&`.
- Template argument deduction/substitution in `std::invoke_result` fails with `no type named 'type'`.

---

## 3. The Working Solution

To successfully bind `std::format` with `std::bind_front`, two requirements must be satisfied:

1. **Construct `std::format_string` at the bind site:**
   Pass `std::format_string< ... >{ "..." }` directly to `std::bind_front`. The literal is evaluated in a constant-expression context when constructing the temporary. Because `std::basic_format_string`'s copy and move constructors are `constexpr` (not `consteval`), the binder safely stores and moves/copies the format string at runtime without triggering immediate escalation.

2. **Use reference types in the template instantiation:**
   Instantiate `format` with `const T&` (e.g. `format< const int&, const int& >`).
   By reference collapsing:
   `const int& &&` $\rightarrow$ `const int&`.
   The resulting parameter type `const int&` binds seamlessly to both the binder's internal member (lvalue `int&`) and the call-site argument (rvalue `12` or lvalue).

### Validated Code Example

```cpp
// 1. Partial binding: format string + leading argument (e.g. color code)
using binary_formatter = string ( * )( std::format_string< const int&, const int& >, const int&, const int& );
auto binary_pointer = static_cast< binary_formatter >( format< const int&, const int& > );

auto bound_binary = bind_front( binary_pointer, std::format_string< const int&, const int& >{ "\033[{};5;{}m" }, 38 );
string result = bound_binary( 12 ); // "\033[38;5;12m"

// 2. Unary binding: format string only
using unary_formatter = string ( * )( std::format_string< const int& >, const int& );
auto unary_pointer = static_cast< unary_formatter >( format< const int& > );

auto bound_unary = bind_front( unary_pointer, std::format_string< const int& >{ "\033[38;5;{}m" } );
string result_unary = bound_unary( 12 ); // "\033[38;5;12m"
```

Verified in `tests/adhoc/0011_bind_format.cpp`.

---

## 4. Alternative for Dynamic Format Strings

If the format string itself is only known at runtime (a dynamic `std::string` or `std::string_view`):
- `std::format` cannot be used directly because compile-time format string validation is mandatory.
- The standard mechanism is `std::vformat` with `std::make_format_args`:

```cpp
auto dynamic_format = [ ]( string_view fmt, auto&&... args ) {
    return std::vformat( fmt, std::make_format_args( args... ) );
};

auto bound_dynamic = bind_front( dynamic_format, "\033[{};5;{}m", 38 );
string dynamic_result = bound_dynamic( 12 ); // "\033[38;5;12m"
```
