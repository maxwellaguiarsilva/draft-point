# Range Pipeline Modernization (C++26)

## 1. Context and Problem Statement

The project originated on Termux (Android), where the NDK libstdc++ was severely limited: no `std::ranges::to`, no range adaptor closures, no `std::views::enumerate`, etc. To compensate, the `sak` library built a parallel pipeline infrastructure: a greedy `sak::operator|` (in `include/sak/geometry/point.hpp`, lines 108-129) that intercepted any invocable, plus a `to_point` proxy for materialization.

After migrating to Arch Linux with GCC 16 (full C++23/C++26 support), this legacy infrastructure collides with the native STL. The symptom is an `ambiguous overload` error (e.g., `source/draft_point.cpp:269`).

### Root Cause (Verified)

The ambiguity is triggered by a precise chain:

1. `sak::operator|(range, invocable)` (overload 3, the non-point transform shortcut) is a greedy template that duplicates `std::views::transform`.
2. It is found via **ADL** whenever a `sak` type appears in the *types* of either operand (e.g., a `std::_Binder` that names `sak::point` as a template argument, a `std::vector<sak::point>`, etc.).
3. It is viable whenever the RHS callable is invocable on the element type. When the elements are themselves ranges (e.g., `sak::point`, `std::string`), std range-adaptor closures like `to<vector>()` *are* invocable on them.
4. Simultaneously, `std::ranges::operator|(range, closure)` is found via the closure's namespace.
5. Both are viable templates, neither more specialized → **ambiguous**.

This was confirmed by reproduction: the HEAD version (using a lambda whose type does not mention `sak`) compiles; replacing the lambda with `bind_front(create_color<>)` (whose `_Binder` type names `sak::point`) reproduces the exact error. The collision is therefore *incidental and fragile* — it depends on invisible type details.

The project's own documentation (`docs/agent/analyses/cpp/color-generation-implementation.md`, section 5) already codified a workaround ("call `to()` as a function, never piped"), proving this fragility had bitten before.

---

## 2. Core Philosophy (Decisions)

These decisions are final and guide all implementation work.

### 2.1 DDD & Visual Purity
Business logic must remain clean and dimension-agnostic (`point | sin | to`). Explicit type declarations inside pipelines (e.g., `std::ranges::to<vec3>()`) contaminate the formula and must be avoided whenever implicit deduction is possible. Type information should flow from the assignment context, not be repeated mid-pipeline.

### 2.2 Zero-Cost Abstractions (Niebloids over Function Pointers)
Mathematical and functional utilities must remain **Niebloids** (function objects / empty structs with `operator()`), never function pointers.

**Rationale — The Overload Set Problem:**
`std::sin` is not a function; it is an *overload set* (`sin(float)`, `sin(double)`, ...). C++ cannot pass an overload set as a template argument — it requires a single concrete type. One could force resolution by demanding a function pointer with a signature derived from the range's element type (a "non-deduced context"):

```cpp
template< input_range R >
auto operator|( R&& r, range_value_t<R>(*)(range_value_t<R>) );
```

This is theoretically valid and the language supports it, but it is rejected because:
- **It breaks `constexpr` and inlining:** a function pointer is an opaque runtime indirection; the compiler cannot inline it or vectorize the loop.
- **It is illegal for STL math functions since C++20:** taking the address of `std::sin` et al. is ill-formed (the standard reserves the right to implement them as macros/intrinsics).
- **It rejects implicit conversions:** an `int` range cannot use a `double(double)` function via a pointer signature.

A Niebloid (a `struct` with a templated `operator()`) solves all three: it is a single concrete type (deduction succeeds), the overload selection is *deferred* until the element is actually available inside `transform`, and the compiler inlines perfectly. This is why the STL itself uses Niebloids (`std::ranges::transform`, `std::ranges::less`, ...).

**Conclusion:** Rebuilding math functions as Niebloids is not a workaround — it is the canonical C++ way to make functions first-class pipeline citizens. The existing macro infrastructure in `math.hpp` is the correct tool to keep this cost near zero.

### 2.3 Native STL Integration (Extend, Don't Bypass)
The architecture must evolve from "bypassing the STL" (Termux era) to "extending the STL" using `std::ranges::range_adaptor_closure` (a C++23 feature, fully available in this C++26 project). Custom global `operator|` overloads are forbidden; all `sak` pipeable utilities must be native range adaptor closures so the STL's own `operator|` routes them.

### 2.4 Opt-In, Not Override
`sak` extensions coexist with the STL via disciplined `__using` imports. The developer explicitly chooses which vocabulary to import:
- `__using(::std::ranges::, to)` → explicit type required (`| to<vec3>()`).
- `__using(::sak::ranges::, to)` → implicit deduction proxy (`| to`).

No `using namespace` directives; no global pollution.

---

## 3. Key Technical Decisions

### 3.1 The Materializer: `sak::ranges::to` (replaces `to_point`)

**Decision:** Delete `to_point`. Replace it with a universal, type-agnostic proxy named `sak::ranges::to`.

**Why `to_point` is retired:**
- The name is contradictory: the proxy is generic (it defers type deduction for *any* target), yet it is named after a single class.
- It was born as a Termux workaround for `std::ranges::to` not existing / not supporting `sak::point`.

**Why a proxy is still needed (not a workaround, but a DDD tool):**
`std::ranges::to<T>()` *requires* the target type `T` to be named in the pipeline. This contaminates the formula and breaks dimension-agnosticism. The `sak::ranges::to` proxy defers the type decision to the assignment site via a templated implicit conversion operator — something the STL cannot do. This is a genuine domain improvement, not a compatibility hack.

**Implementation shape:**
```cpp
template< input_range R >
struct __to_proxy {
    R m_range;
    template< typename T >
    constexpr operator T() && {
        return std::ranges::to<T>( std::forward<R>(m_range) );
    }
};

struct to_closure : std::ranges::range_adaptor_closure<to_closure> {
    template< input_range R >
    constexpr auto operator()( R&& r ) const {
        return __to_proxy<R>{ std::forward<R>(r) };
    }
};

inline constexpr to_closure to;
```

**Prerequisite:** For `std::ranges::to<T>` to work with `sak::point`, `point` must gain a `std::from_range_t` constructor (see 3.3).

### 3.2 Math Niebloids as Native Range Adaptors

**Decision:** Upgrade the macro-generated Niebloids in `math.hpp` to inherit from `std::ranges::range_adaptor_closure` and expose a range overload.

**Implementation shape (macro evolution):**
```cpp
#define __sak_unary_niebloid( a_name, a_expr ) \
struct __##a_name : std::ranges::range_adaptor_closure<__##a_name> \
{ \
    constexpr auto operator()( auto v ) const noexcept { return a_expr; } \
    template< input_range R > \
    constexpr auto operator()( R&& r ) const { \
        return std::views::transform( std::forward<R>(r), *this ); \
    } \
}; \
inline constexpr auto a_name = __##a_name{};
```

**Result:** `range | sak::math::sin` resolves natively via the STL `operator|`. No custom pipe operator is involved. The scalar form `sin(0.5f)` still works. Inlining and `constexpr` are preserved.

### 3.3 `sak::point` as a First-Class C++23 Range Consumer

**Decision:** Add a range constructor to `sak::point` using the `std::from_range_t` protocol.

```cpp
template< input_range R >
constexpr point( std::from_range_t, R&& r ) : super_type{} {
    std::ranges::copy( r, this->begin() );
}
```

**Result:** `std::ranges::to<vec3>()` works natively (used internally by the `sak::ranges::to` proxy). `sak::point` becomes a standard-conforming range consumer.

### 3.4 Eradication of Legacy Pipe Operators

**Decision:** Delete the three `sak::operator|` overloads in `point.hpp` (lines 108-129):
1. `range | __to_point` — superseded by `sak::ranges::to` (native closure).
2. `point | invocable` — superseded by Niebloid closures (`point | sin`).
3. `range | invocable` (non-point) — unused in active code; pure duplication of `std::views::transform`; the sole source of the ambiguity.

**Result:** Total elimination of ADL pollution. The C++23 STL owns all pipeline routing.

---

## 4. Implementation Roadmap

| Step | Target | Action |
|------|--------|--------|
| 1 | `sak/math/math.hpp` | Upgrade Niebloid macros: inherit `range_adaptor_closure`, add range `operator()`. |
| 2 | `sak/point.hpp` | Add `std::from_range_t` constructor to `sak::point`. |
| 3 | `sak/ranges/` (new header) | Create `sak::ranges::to` closure + `__to_proxy` with templated conversion operator. |
| 4 | `sak/geometry/point.hpp` | Delete `to_point`, `__to_point`, `__point_from`, and the three `operator|` overloads (lines 88-129). |
| 5 | Active code (`source/`, `tests/adhoc/`) | Migrate call sites: `| to_point` → `| to` (imported from `sak::ranges`); `| sin | to_point` → `| sin | to`. Remove documented workarounds (e.g., `to<vector<vec3>>(range)` function-form calls) where the pipe form is now safe. |
| 6 | Tests (`tests/sak/`) | Update unit tests to consume the new closures by design (they validate `sak` helpers). |

### Validation Criteria
- `cpp-compile` (MCP tool, compiles the entire project) passes with `-Wall -Wextra -Werror`.
- All `tests/sak/` grouped tests compile and pass.
- All `tests/adhoc/rendering/` demos (0005, 0007, 0009) compile.
- No remaining `sak::operator|` free overloads in `point.hpp`.
- `point | sin | to` and `count_to(n) | transform(f) | to` both compile without ambiguity.

---

## 5. Expected Outcome

The codebase compiles cleanly on Arch Linux (GCC 16, C++26). Business-logic syntax becomes cleaner (`point | sin | to` instead of `point | sin | to_point`), and is powered 100% by native STL mechanisms (`range_adaptor_closure` + the STL `operator|`). The Termux-era parallel pipeline is fully retired. Ambiguities are eliminated by construction, not by convention.
