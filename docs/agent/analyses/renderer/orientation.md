# Orientation

Import: [3D Renderer](index.md)

## Scope

- Stage 5: give objects a face orientation computed on the CPU and consumed as a matrix by the GPU.
- Records the answer to the library question that this plan depends on: whether `sak` already provides quaternion multiplication.

## Facts

- `hamilton_product` does not exist in `sak`. A repository search for `hamilton` and `quaternion` under `include/` and `docs/` returned no matches. The check was run with `rg -L "hamilton|quaternion" include/ docs/`.
- No quaternion type and no matrix type exist in `sak`.
- The only rotation available is `sak::math::rotate`, a Rodrigues rotation of a vector around an axis by an angle, in `include/sak/math/vector.hpp`. It returns a lazy vector expression and is not a matrix builder.
- Evidence: `include/sak/math/vector.hpp` `__rotate`; the absent identifiers above.

## Decisions

- Each object carries a 4D orientation value (`point< float, 4 >` equivalent to `vec4`) as its quaternion.
- The CPU composes the rotation and converts it to a `mat3` before flushing.
- The GPU receives the resulting `mat3` and the shader never sees a quaternion.

## Open Questions

- How a `mat3` crosses a buffer that only carries floats, given the flat-payload rule of [Agnostic Pipeline](agnostic-pipeline.md). Three basis vectors, sent as nine floats, are the representation that matches the existing accessor macros; the alternative is a new `mat3` accessor block in the shader template.
- Row-major or column-major in the payload. The shader assembles structs by hand, so the convention must be written down once next to the accessor macro that builds the matrix.
- Whether `hamilton_product` belongs to `sak::math` as a binary niebloid over two 4D vectors, and how it stays symmetric. The product is a sum of basis products; writing it as one expression over rotated views of the two operands is the form the project requires, and it is the first thing to attempt before any component expansion is considered.
- Whether orientation is applied to the ray before figure evaluation or to the resulting hit point. Rotating the ray is cheaper per figure, and it keeps every figure body free of orientation logic.

## Exclusions

- No quaternion normalization policy is decided here.
- The stage does not add interpolation of orientations between frames; objects are either static or oriented per frame.

## Relations

- Requires the figure contract of [Agnostic Pipeline](agnostic-pipeline.md), which reserves the orientation slot in the base payload.
- Applies to the meshes of [Adaptive Mesh](adaptive-mesh.md) and to the objects tracked by [Spatial Graph](spatial-graph.md).
