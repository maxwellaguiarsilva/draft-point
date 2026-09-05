# Status

## Planning: Range Pipeline Modernization (C++26) — Multi-Session

### Objective
Plan the cross-repo refactoring described in [range-pipeline-cpp26.md](architecture/range-pipeline-cpp26.md). The blueprint is high-level; planning is split into sessions because the work spans `draft-point` and the linked `sak` library (in `project-mcp-tools`).

### Progress
- **Session 1 (scope discovery) — done.** Mapped both repos; found 6 gaps between blueprint and code.
- **Session 2 (`sak`-side audit) — done.** Inventoried 8 free `operator|` overloads (blueprint cited 3), 2 legacy materializers, 4 legacy closures needing `range_adaptor_closure` migration, 4 internal consumer sites, and the full niebloid classification.
- **Session 3 (call-site audit, `draft-point`) — done.** Inventoried 10 files with legacy call sites and 9 clean files. Totals: 24 `to_point`, 6 `to_array`, 7 `line_to`, 3 `rotated`, 1 `enumerate`, 1 `sin`, 2 raw-lambda pipes, 2 binder pipes, 2 `to<vector>()` workarounds. Full per-file migration list + cross-cutting notes (import namespace shift, proxy needs explicit LHS type, std/sak `to` clash, silent lambda/binder breakers, decision dependencies).
- **Session 4 (synthesis) — done.** Revised blueprint §4 into a dependency-sequenced, executable roadmap. Resolved the 5 open decisions: (1) binary niebloids pipeable via partial application; (2) `operators.hpp` out of scope; (3) `to_array` retired into `sak::ranges::to` via `__to_impl<std::array>` class-template partial specialization (feasibility confirmed — `to_array` already used the same proxy pattern); (4) `cartesian_product` upgraded to closure; (5) one atomic sak-side commit.
- **Session 5 (implementation) — in progress (context window hit).** Key discovery: **the sak-side (§3) is ALREADY fully implemented and committed** in `project-mcp-tools` (HEAD `7327799` "add eager_transform and lazy_transform aliases to end the transform collision"). Verified against plan §3: `ranges/to.hpp` exists (universal materializer, `__to_impl<std::array>` specialization), `math/math.hpp` has all closures (`negate`, `tanh`, `clamp`, `between`, `is_multiple`, `atan2`, `pow`, 14 std-wraps), `geometry/point.hpp` has the `from_range_t` ctor + `*this | negate | to`, `geometry/line_view.hpp` has `line_to` closure + `| abs | to`/`| sign | to`/`lazy_transform(m_walker, bind_back(...)) | to`, `views/rotated.hpp`/`enumerate.hpp`/`cartesian_product.hpp` are closures, `ranges/to_array.hpp` is **deleted**. Draft-point already has `65f3a86 "use lazy_transform at transform call sites"` committed. **So: sak-side commit = done, no action needed. Only the draft-point side (§4) remains.** Draft-point working tree is currently **dirty** with the partial migration below.
- **Session 6 (validation) — the "sak-side done" conclusion was WRONG. The sak-side implementation is broken and does not compile against the migrated call sites.** `cpp-compile` fails across the board with two distinct defects. Full repro evidence and fix direction in the handoff below. **The sak repo MUST be touched after all** (contradicts the Session 5 "do not touch" note). All §4 draft-point edits are applied and clean (`grep to_point/to_array` empty); they are correct per the plan but cannot compile until the sak-side defects are fixed.
- **Session 7 (decision + final inventory) — done.** Confirmed both defects with minimal repros vs sak HEAD (`repro4.cpp` = Defect A, `repro11.cpp` = Defect B; new `tan_test.cpp` proves `tan` **also** collides — the Session 6 list was missing it, so the affected set is **17 names**). `repro7.cpp` confirms `clamp( lo, hi ) | to` and `lazy_transform | to` work (only unary closure pipes break). **User decision on Fix B (overrides Session 6's "descriptive-only" proposal):** keep the short name but give it a `sak_` prefix AND add a descriptive-English alias — every colliding closure gets **two objects**, e.g. `abs = sak_abs + absolute`, `sin = sak_sin + sine`, etc. Non-colliding closures keep plain names. **Two new discoveries not covered by the old §4 list:** (1) `sak/math/vector.hpp:76-77` calls `cos( angle )`/`sin( angle )` inside `namespace sak::math` → resolves to the closures (3rd internal consumer); after rename it would silently fall back to global `::cos`/`::sin`, so it must use `sak_cos`/`sak_sin` (local vars are literally named `cosine`/`sine`, so the aliases can't be used there). (2) `source/draft_point.cpp:256` imports `sak::math:: sin, cos` (used at 178, 188) — was "done/no-change" in the old handoff, now MUST become `sine`/`cosine`. Dead std imports to clean: `0007:44-45` and `0009:40-41` (`pow` already unused, `exp` becomes dead once renamed). No colliding-closure usage in `include/game/`, `include/tui/`, `test_sak_geometry.cpp`, `test_sak_ranges.cpp`, `tests/sak/ranges/test_sak_ranges_views.cpp`.
- **Session 8 (execution & completion) — done.** Applied sak-side fixes (Fix A: constrained scalar overloads in math.hpp macros, Fix B: renamed 17 colliding math closures to `sak_<short>` + descriptive English aliases). Updated vector.hpp and line_view.hpp. Updated draft-point call sites (0005, 0007, 0009 demos, draft_point.cpp, test_sak_math.cpp) to descriptive aliases. Verified full compilation with `cpp-compile`, executed all 6 sak test binaries successfully, verified clean static analysis with `cpp-analyze`, and committed both repositories.

### Next (handoff for a fresh chat) — FULL execution checklist (Session 8)

**State:** draft-point working tree is dirty with the **complete §4 migration already applied** (verified against plan §4: all `to_point`/`to_array`/binder/lambda/`to<vector>()` edits done, `grep to_point|to_array` empty — see the last "Operation" section in git status below for the pre-rename state). The sak repo (`project-mcp-tools`) is clean at `7327799` and is **broken** (Defects A+B). Order of operations: **fix sak FIRST, then the rename call-site updates, then validate, then commit sak → draft-point.**

---

## 1. Sak-side fix (`project-mcp-tools/sak-lib/cpp/sak/`) — one atomic commit

### 1.1 `math/math.hpp` — Fix A (constrain scalar overloads)

The concept `is_arithmetic` already exists (lines 81-82). Constrain the scalar `operator()` in **all six** locations so an lvalue range no longer makes both overloads viable (SFINAE: scalar overload drops out for ranges, range overload drops out for scalars):

| Location | Current | New |
|---|---|---|
| `__935812590_unary_niebloid` macro | `constexpr auto operator ( ) ( auto a_value ) const noexcept` | `template< is_arithmetic t_value > constexpr auto operator ( ) ( t_value a_value ) const noexcept` |
| `__935812590_unary` macro | same | same |
| `__935812590_binary` macro | `template< typename t_left, typename t_right > constexpr auto operator ( ) ( t_left a_left, t_right a_right ) const noexcept` | `template< is_arithmetic t_left, typename t_right > …` (constrain the FIRST arg only — `t_right` is the bound scalar in both forms) |
| `negate` | via unary macro | covered |
| `between` scalar | `template< typename t_value, typename t_start, typename t_end >` | `template< is_arithmetic t_value, typename t_start, typename t_end >` |
| `clamp` scalar | `template< typename t_value, typename t_low, typename t_high >` | `template< is_arithmetic t_value, typename t_low, typename t_high >` |

### 1.2 `math/math.hpp` — Fix B (rename colliding closures: `sak_<short>` + descriptive alias)

**User decision (Session 7):** every colliding closure gets TWO `inline constexpr` objects of the same closure type — the short name prefixed `sak_` **and** a descriptive-English alias. Struct names keep `__<name>` (reserved-name convention unchanged). Non-colliding closures keep their plain name.

**Mechanic:** change all three macros to emit the **struct only** (drop the trailing `inline constexpr auto a_name = __##a_name{ };`), then add explicit object/alias lines after each invocation. (`is_even`/`is_odd` scalar bodies call `is_multiple( a_value, 2 )` — `is_multiple` is NOT renamed, and its scalar overload now takes `is_arithmetic`, so the arithmetic element still matches.)

**Full mapping (17 colliding closures):**

| short | `sak_` object | descriptive alias |
|---|---|---|
| abs | sak_abs | absolute |
| sin | sak_sin | sine |
| cos | sak_cos | cosine |
| tan | sak_tan | tangent |
| tanh | sak_tanh | hyperbolic_tangent |
| exp | sak_exp | exponential |
| log | sak_log | natural_logarithm |
| log2 | sak_log2 | logarithm_base_two |
| floor | sak_floor | round_down |
| ceil | sak_ceil | ceiling |
| round | sak_round | round_to_nearest |
| trunc | sak_trunc | truncate |
| pow | sak_pow | power |
| atan | sak_atan | arctangent |
| atan2 | sak_atan2 | arctangent2 |
| acos | sak_acos | arccosine |
| asin | sak_asin | arcsine |

Explicit object lines after macro invocations:
- Plain name kept: `negate, square, square_root, is_even, is_odd, sign, is_multiple` → `inline constexpr auto negate = __negate{ };` etc.
- Colliding (two lines each): e.g. `inline constexpr auto sak_abs = __abs{ };` + `inline constexpr auto absolute = __abs{ };`.

### 1.3 `math/vector.hpp` (internal consumer — lines 76-77)

Local vars are named `cosine`/`sine`, which would shadow the aliases — **must use the `sak_` forms**:
- `const float cosine = cos( angle );` → `const float cosine = sak_cos( angle );`
- `const float sine = sin( angle );` → `const float sine = sak_sin( angle );`

### 1.4 `geometry/line_view.hpp` (internal consumer — `abs`)

- line 48: `,abs` → `,absolute`
- line 69: `m_difference | abs | to` → `m_difference | absolute | to`
- line 70: `m_difference | sign | to` — **unchanged** (`sign` not renamed)

### 1.5 `geometry/point.hpp` — **no change** (internal pipe uses `negate`, not renamed).

### 1.6 Re-verify

`grep -rnE '\b(sin|cos|tan|tanh|exp|log|log2|floor|round|trunc|ceil|pow|atan2|atan|acos|asin|abs)\b' sak-lib/cpp/sak/` → only the struct definitions / `::std::` bodies inside `math.hpp`, plus the intended `sak_cos`/`sak_sin`/`absolute`/`sine` uses. **No sak header may reference a deleted plain closure name as an object.**

---

## 2. Draft-point-side update (apply on top of the already-dirty §4 tree) — rename to descriptive aliases

### 2.1 `source/draft_point.cpp`  ← **NOT in the old §4 list, discovered in Session 7**
- line 256: `__using( ::sak::math::, sin, cos, between )` → `__using( ::sak::math::, sine, cosine, between )`
- line 178: `vec3 position{ cos( element_angle ), sin( element_angle ), sin( element_angle ) };` → `{ cosine( element_angle ), sine( element_angle ), sine( element_angle ) }`
- line 188: `phase * cos( ( m_angle + index * step ) * m_total )` → `phase * cosine( ( m_angle + index * step ) * m_total )`

### 2.2 `tests/adhoc/rendering/0005_shadertoy_demo.cpp`
- line 40: `__using( ::sak::math::, sum, dot, exp, tanh )` → `sum, dot, exponential, hyperbolic_tangent`
- line 94: `| exp | to;` → `| exponential | to;`
- line 96: `e *= exp( -4.0f * l );` → `e *= exponential( -4.0f * l );`
- line 98: `| tanh | to;` → `| hyperbolic_tangent | to;`
- (std `sin`/`cos`/`abs` imports at lines 38, 54 and their scalar uses at 78/89/90 — **unchanged**, they are `::std` functions)

### 2.3 `tests/adhoc/rendering/0007_raymarching_primitives.cpp`
- lines 44-45: remove dead `,pow` and `,exp` from the `::std` import block (keep `min, max, floor`)
- line 51: `__using( ::sak::math::, sum, sin, cos, abs, clamp, dot, length, normalize, cross )` → `sum, sine, cosine, absolute, clamp, dot, length, normalize, cross`
- line 142: `abs( hit[ 0 ] )` → `absolute( hit[ 0 ] )`
- line 173: `__using( ::sak::math::, sin, cos, clamp, exp )` → `sine, cosine, clamp, exponential`
- line 203: `sin( angle )` → `sine( angle )`; `cos( angle )` → `cosine( angle )`
- line 232: `| sin | to;` → `| sine | to;`
- line 239: `1.0f - exp( -0.0001f * distance )` → `1.0f - exponential( -0.0001f * distance )`

### 2.4 `tests/adhoc/rendering/0009_raytracing_primitives.cpp`
- lines 40-41: remove dead `,pow` and `,exp` from the `::std` import block (keep `min, max, floor`)
- lines 48-50: `,sin ,cos ,abs` in the `::sak::math::` block → `,sine ,cosine ,absolute`
- line 104: `abs( ray_direction[ 1 ] )` → `absolute( ray_direction[ 1 ] )`
- line 152: `__using( ::sak::math::, sin, cos, clamp, exp )` → `sine, cosine, clamp, exponential`
- line 167: `sin( -angle )` → `sine( -angle )`; `cos( -angle )` → `cosine( -angle )`
- line 205: `1.0f - exp( -0.0001f * distance )` → `1.0f - exponential( -0.0001f * distance )`

### 2.5 `tests/sak/test_sak_math.cpp`
- line 57: `,abs` in the `::sak::math::` block → `,absolute`
- line 211: `abs( preserved - length( source ) )` → `absolute( preserved - length( source ) )`

### 2.6 No rename changes needed (verified no colliding-closure usage): `source/tui/terminal.cpp`, `source/tui/renderer.cpp`, `tests/sak/test_sak_geometry.cpp`, `tests/sak/test_sak_ranges.cpp`, `tests/sak/ranges/test_sak_ranges_views.cpp`, all of `include/game/` + `include/tui/`.

---

## 3. Validation (§5) — run from `draft-point` AFTER both sides are edited

- `cpp-compile` (MCP tool) with `-Wall -Wextra -Werror` — must pass
- Run all `tests/sak/` grouped test binaries (they must pass)
- Adhoc demos 0005/0007/0009 must compile
- `grep -rn "to_point\|to_array" source/ tests/` → empty; no `to< vector >( )` left
- `grep -rn "__using( ::sak::math::, .*\b(sin|cos|abs|exp|tanh|floor|log|log2|round|trunc|ceil|pow|atan2|atan|acos|asin|tan)\b" source/ tests/` → empty (no plain colliding closure imported)
- `cpp-code-verifier` + `cpp-analyze` clean on changed files
- Blueprint showcases compile: `point | sine | to`, `vec | clamp(lo, hi) | to`, `range | rotated`, `range | rotated(n)`

## 4. Commit sequence (en-US, lowercase messages)

1. **`project-mcp-tools` first:** one atomic commit (math.hpp Fix A+B + vector.hpp + line_view.hpp). Suggested: `rename colliding math closures to sak_ prefix and descriptive aliases`.
2. **`draft-point` second:** one commit with the whole dirty tree (§4 + rename updates). Suggested: `migrate call sites to sak::ranges::to and adaptor closures`.

---

## Background (Sessions 5-6, kept for reference)

**Defect A — scalar/range overload ambiguity (breaks ALL unary closures as pipes).** `__935812590_unary_niebloid` / `__935812590_unary` give each closure scalar `operator()(auto)` + `operator()(t_range&&) requires input_range`. For an **lvalue range** both are viable and ambiguous → the STL `operator|` constraint `__adaptor_invocable` fails → `no match for operator|`. Repro: `repro4.cpp` (`sak::math::abs( point )`). `clamp( lo, hi ) | to` and `lazy_transform(...) | to` DO work (single range-only `operator()`).

**Defect B — name collision with global C math functions (breaks unqualified imports).** glibc+GCC put `::sin … ::asin` (+ `::abs`) in the global namespace. A `using`-declaration does NOT hide outer-scope names, and a function object colliding with a global FUNCTION is ambiguous (unlike functions, which merge). Repro: `repro11.cpp` (`abs`), `tan_test.cpp` (`tan` — missing from the Session 6 list). **Affected set = 17 names** (the table in §1.2).

**Why it compiled before:** pre-migration sak (1a9515b) had scalar-only niebloids used only in scalar form; the collision is new with the closure upgrade (88179e3). Post-migration draft-point HEAD never compiled either (deleted `to_point`) — the cross-repo gap was never closed.

**Repro files (in `/tmp/opencode/`):** `repro3.cpp`/`repro11.cpp` (Defect B), `repro4.cpp`/`repro5.cpp` (Defect A), `repro6/7.cpp` (clamp + lazy_transform work), `tan_test.cpp` (tan collides), `orig/0007_orig.cpp` + `originc/sak` @1a9515b (pre-migration proof).

---

## Operation: Group Hierarchical Tests by Folder

### Objective
The hierarchical tests under `tests/sak/` are currently 1-to-1 with each `include/sak/**/*.hpp` header. The goal is to group them by folder: every object inside a folder (e.g. `include/sak/pattern/`) is tested by a single grouped test inside that hierarchy (e.g. `tests/sak/pattern/test_sak_pattern.cpp`). Adhoc tests (`tests/adhoc/`) are not affected.

### Progress

1. **Old tests moved to `.old/tests/`** (gitignored location, per `.gitignore` "old files/directories" section) to keep the working tree clean and allow a controlled, folder-by-folder transfer:
   - `.old/tests/sak/geometry/` (3 files: line_view, point, rectangle)
   - `.old/tests/sak/math/` (2 files: sum, vector)
   - `.old/tests/sak/pattern/` (4 files: dispatcher, to_number, tupled, value_or)
   - `.old/tests/sak/ranges/` (4 files: chunk, count_to, fold_left_first, operators)
   - `.old/tests/sak/ranges/views/` (3 files: enumerate, join_with, rotated)
   - `.old/tests/sak/test_sak_string.cpp`

2. **New grouped test files created** via `create_test` with `flg_adhoc = false` (CLI: `./scripts/cpp-create-test --hierarchy <h> --no-flg-adhoc`):
   | Hierarchy | Created file |
   |---|---|
   | `sak` | `tests/test_sak.cpp` |
   | `sak/geometry` | `tests/sak/test_sak_geometry.cpp` |
   | `sak/math` | `tests/sak/test_sak_math.cpp` |
   | `sak/pattern` | `tests/sak/test_sak_pattern.cpp` |
   | `sak/ranges` | `tests/sak/test_sak_ranges.cpp` |
   | `sak/ranges/views` | `tests/sak/ranges/test_sak_ranges_views.cpp` |

   **Decision:** keep the `create_test` locations as-is (test for hierarchy X lives in the parent folder). No relocation to mirror `include/sak/`.
   **Note:** the generated files carry unrendered `{{...}}` template placeholders in the file header (`file_created_at_year`, `file_author_name`, etc.). They were replaced with proper headers during the transfer. This is a tool defect in `project-mcp-tools` (the `create_test` context lacks the `file-*` keys expected by the header template) — worth fixing upstream.

### Result (completed)

1. **All 6 grouped test files created, merged, and verified:**
   | File | Merged unit tests |
   |---|---|
   | `tests/test_sak.cpp` | `sak/string` (to_lower_case / to_upper_case). Trivial macros/utilities (`__using`, `ensure`, `byte`, `default_ctc_dtc`) intentionally skipped. |
   | `tests/sak/test_sak_geometry.cpp` | point, line_view, rectangle |
   | `tests/sak/test_sak_math.cpp` | sum, vector |
   | `tests/sak/test_sak_pattern.cpp` | dispatcher, to_number, tupled, value_or |
   | `tests/sak/test_sak_ranges.cpp` | chunk, count_to, fold_left_first, operators |
   | `tests/sak/ranges/test_sak_ranges_views.cpp` | enumerate, rotated |

2. **Library fix (in `project-mcp-tools`, sak-lib):** combining `chunk.hpp` and `operators.hpp` in one TU exposed a latent `transform` collision — `chunk.hpp` imported `std::views::transform` and `operators.hpp` imported `std::ranges::transform`, both into `namespace sak::ranges`. Fixed `chunk.hpp`: its single `transform(...)` call is now qualified `::std::views::transform` and `transform` was dropped from its `__using` import list (behavior unchanged).

3. **Old per-header test files deleted** after each transfer was compiled and executed successfully (backup `.old/` cleaned up; directory removed).

4. **Verification:** `cpp-compile` (with `-Wall -Wextra -Werror`) compiles cleanly; all 6 grouped test binaries run and pass; `cpp-code-verifier` reports no formatting violations; `cpp-analyze` (static analysis + auto-fix) completes successfully.

5. **Git index synced** (`git add -A tests/`) because the cpp tooling lists files via `git ls-files --cached` and the plain `mv` left stale tracked entries. Git detected the transfers as renames.

---

## Change Plan: Test 0007 (`tests/adhoc/0007_raymarching_primitives/0007_raymarching_primitives.cpp`)

### Objective
Refactor the raymarching primitives test (`0007_raymarching_primitives`) so that `position` (`m_position`) is a property of the abstract base class `primitive` rather than individual derived classes (`sphere`, `torus`, `plane`), adhering to the project's standard architecture.

### Planned Changes

1. **Abstract Base Class `primitive`**:
   - Add `vec3 m_position;` member variable.
   - Update the constructor to accept `vec3 position` and `float material_id`:
     ```cpp
     struct primitive
     {
         vec3 m_position;
         float m_material_id;

         primitive( vec3 position, float material_id )
             : m_position( position ), m_material_id( material_id ) {}
         auto position( ) const -> vec3 { return m_position; }
         auto material_id( ) const -> float { return m_material_id; }
         virtual auto sdf( vec3 point ) const -> float = 0;
         virtual ~primitive( ) = default;
     };
     ```

2. **Derived Class `plane`**:
   - Constructor takes `float height, float material_id`, passing `vec3{ 0.0f, height, 0.0f }` to the `primitive` base constructor.
   - In `sdf( vec3 point )`, compute distance using `m_position[ 1 ]` (or `y` component):
     ```cpp
     struct plane final : primitive
     {
         plane( float height, float material_id )
             : primitive( vec3{ 0.0f, height, 0.0f }, material_id )
         { }

         auto sdf( vec3 point ) const -> float override
         {
             return point[ 1 ] - m_position[ 1 ];
         }
     };
     ```

3. **Derived Class `sphere`**:
   - Remove local `vec3 m_position;`.
   - Update constructor to pass position to `primitive`:
     ```cpp
     struct sphere final : primitive
     {
         float m_radius;

         sphere( vec3 position, float radius, float material_id )
             : primitive( position, material_id )
             , m_radius( radius )
         { }

         auto sdf( vec3 point ) const -> float override
         {
             return length( point - m_position ) - m_radius;
         }
     };
     ```

4. **Derived Class `torus`**:
   - Remove local `vec3 m_position;`.
   - Update constructor to pass position to `primitive`:
     ```cpp
     struct torus final : primitive
     {
         vec2 m_radii;

         torus( vec3 position, vec2 radii, float material_id )
             : primitive( position, material_id )
             , m_radii( radii )
         { }

         auto sdf( vec3 point ) const -> float override
         {
             auto local = point - m_position;
             vec2 relative_2d_position{ length( vec2{ local } ) - m_radii[ 0 ], local[ 2 ] };
             return length( relative_2d_position ) - m_radii[ 1 ];
         }
     };
     ```

### Verification
- Compile and run test 0007 or run project compilation tools to ensure correctness.

