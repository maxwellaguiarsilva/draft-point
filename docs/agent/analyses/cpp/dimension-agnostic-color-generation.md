# Dimension-Agnostic Color Generation

Design and analysis part of the color generation proposal, split into this
document and [color-generation-implementation.md](color-generation-implementation.md).
This part covers §1 (problem), §2 (approach) and §3 (`sak` infrastructure).

## 1. Problem Statement

The adhoc rendering test `tests/adhoc/rendering/0010_sphere.cpp` hardcodes a fixed-size array of RGB colors (`terminal_colors`) whose length is tightly coupled to the constant `total`. Each color is a permutation of two discrete values (`0.5` and `1.0`) across three channels, yielding `2^3 = 8` entries.

This approach has two limitations:

1. **Coupling:** Changing `total` requires manually rewriting the color array.
2. **Dimension lock-in:** The generation logic is implicitly tied to three dimensions (RGB). Extending it to `vec2` or `vec4` would require duplicating the pattern with a different number of nested loops.

The goal is a `create_color` function that:

- Produces a distinct color for each 1D index by mapping the index into N-dimensional space.
- Is agnostic to the number of dimensions of the point type.
- Requires no changes to `sak/math/math.hpp`.
- Follows the declarative pipeline style mandated by the project style guide.

---

## 2. Approach: Bit Deinterleaving + Spatial Subdivision

The core insight is that a 1D color index can be mapped to an N-dimensional coordinate by **deinterleaving its bits** across dimensions (a Morton/Z-order curve), and each per-dimension channel index can be mapped to a value via a **spatial subdivision sequence** that prioritizes extremes.

This replaces the previous base-N numeral system approach. The key difference: instead of treating the index as a base-`V` number (which produces a counting pattern `000, 100, 010, 110, ...`), bit deinterleaving distributes consecutive indices across the N-dimensional space in a spatially coherent way, and the subdivision sequence ensures the first colors hit the extremes of each channel for maximum visual distinctness.

### 2.1 Bit Deinterleaving (Morton/Z-order Curve)

Given an `index` and `N` dimensions, dimension `d` (0-indexed) collects the bits at positions `d, d+N, d+2N, ...` from the index, packing them into a per-channel index:

```
channel_index = 0
for each source bit at position d, d+N, d+2N, ... (while < 8):
    channel_index |= ((index >> source_bit) & 1) << dest_bit
    dest_bit++
```

For 3 dimensions, this is the classic RGB bit deinterleaving:

- **Red** (d=0): bits 0, 3, 6
- **Green** (d=1): bits 1, 4, 7
- **Blue** (d=2): bits 2, 5

For 2 dimensions:

- **X** (d=0): bits 0, 2, 4, 6
- **Y** (d=1): bits 1, 3, 5, 7

This generalizes to any dimension count without code changes.

### 2.2 Spatial Subdivision Sequence

Each channel index is mapped to a normalized value in `[0, 1]` via a sequence that prioritizes extremes, then fills in midpoints recursively:

| Channel index | Value | Level |
|---|---|---|
| 0 | 0.0 | extremes |
| 1 | 1.0 | extremes |
| 2 | 0.5 | halves |
| 3 | 0.25 | quarters |
| 4 | 0.75 | quarters |
| 5 | 0.125 | eighths |
| 6 | 0.625 | eighths |
| 7 | 0.375 | eighths |
| 8 | 0.875 | eighths |
| ... | ... | ... |

The sequence is defined as:

- Index 0 → `0.0` (minimum extreme)
- Index 1 → `1.0` (maximum extreme)
- Index `k ≥ 2` → base-2 radical inverse (van der Corput) of `k - 1`

The radical inverse of an integer `n` is computed by reversing its bits after the binary point:

```
result = 0
weight = 0.5
while n > 0:
    result += weight * (n & 1)
    n >>= 1
    weight *= 0.5
```

This produces: `radical_inverse(1) = 0.5`, `radical_inverse(2) = 0.25`, `radical_inverse(3) = 0.75`, `radical_inverse(4) = 0.125`, etc.

The benefit of "extremes first" is that the first `2^N` colors are the corners of the N-dimensional unit hypercube — the maximally distinct set of colors. After the corners are exhausted, the sequence subdivides each edge, then each face, then the interior, maintaining maximum distance between consecutive colors.

### 2.3 Worked Example (3D, 8 colors)

For `dimensions = 3`, `min_value = 0.0`, `max_value = 1.0`:

| Index | Binary | Red (bits 0,3,6) | Green (bits 1,4,7) | Blue (bits 2,5) | Color |
|-------|--------|-------------------|---------------------|------------------|-------|
| 0 | 00000000 | 0 → 0.0 | 0 → 0.0 | 0 → 0.0 | `{ 0.0, 0.0, 0.0 }` |
| 1 | 00000001 | 1 → 1.0 | 0 → 0.0 | 0 → 0.0 | `{ 1.0, 0.0, 0.0 }` |
| 2 | 00000010 | 0 → 0.0 | 1 → 1.0 | 0 → 0.0 | `{ 0.0, 1.0, 0.0 }` |
| 3 | 00000011 | 1 → 1.0 | 1 → 1.0 | 0 → 0.0 | `{ 1.0, 1.0, 0.0 }` |
| 4 | 00000100 | 0 → 0.0 | 0 → 0.0 | 1 → 1.0 | `{ 0.0, 0.0, 1.0 }` |
| 5 | 00000101 | 1 → 1.0 | 0 → 0.0 | 1 → 1.0 | `{ 1.0, 0.0, 1.0 }` |
| 6 | 00000110 | 0 → 0.0 | 1 → 1.0 | 1 → 1.0 | `{ 0.0, 1.0, 1.0 }` |
| 7 | 00000111 | 1 → 1.0 | 1 → 1.0 | 1 → 1.0 | `{ 1.0, 1.0, 1.0 }` |

The first 8 colors are the 8 corners of the RGB cube — black, red, green, yellow, blue, magenta, cyan, white. Index 8 subdivides the red channel to 0.5, producing `{ 0.5, 0.0, 0.0 }`.

---

## 3. Leveraging `sak` Infrastructure

### `is_point` concept (`sak/geometry/point.hpp`)

Constrains the template parameter to any `sak::point<T, N>` specialization. This gives the function access to:

- `size()` — returns the number of dimensions at compile time.
- `operator[]` — per-component access (re-exposed from `std::array`).

### `sak::ranges::to` materializer (replaces `to_point`)

The pipeline ends with a materializer that defers the target type to the assignment site:

```cpp
range | to  ->  point<T, N>
```

`to` is `sak::ranges::to`, a native `std::ranges::range_adaptor_closure` that returns a proxy whose templated conversion operator materializes into the point type deduced from the assignment context. It defers naming the dimension count mid-pipeline (the DDD preference, since `std::ranges::to<vec3>()` would require naming it). It replaces the retired `to_point` proxy — see [range-pipeline-cpp26.md](../../architecture/range-pipeline-cpp26.md) §3.1.

### Native STL routing (no custom `operator|`)

`| to` resolves through the STL's own `operator|` because `to` is a `range_adaptor_closure` (C++23) — no `sak::operator|` ADL is involved. The preceding `iota | transform` uses `std::views::transform`; the whole pipeline is native STL routing.

### No `sak/math` modifications required

The mapping uses only bit manipulation (deinterleaving and radical inverse) and existing `sak` primitives (`is_point`, `sak::ranges::to`). No `pow`, `ceil`, or other math niebloids are needed, since the index-to-coordinate mapping is purely bitwise.

### One `sak` prerequisite (already settled)

For the `to` proxy to build a point, `sak::point` must be a standard-conforming range consumer — it gains a `std::from_range_t` constructor (see [range-pipeline-cpp26.md](../../architecture/range-pipeline-cpp26.md) §3.3). This is a settled `sak` change, not a `sak/math` change.
