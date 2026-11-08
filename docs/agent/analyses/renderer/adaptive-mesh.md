# Adaptive Mesh

Import: [3D Renderer](index.md)

## Scope

- Stage 4: apply the analytic rounding of [Rounded Primitives](rounded-primitives.md) to an arbitrary triangle mesh whose edges have different lengths, so every corner rounds proportionally to what fits there.
- The mesh keeps its original polygon count. Rounding is a property of the evaluated surface, not of added geometry.

## Decisions

### Global Proportion

- `environment` carries one global `float border_radius` used as a proportion, `0.1f` meaning ten percent.

### Per-Corner Clamp

- At a vertex shared by three edges, the local rounding radius is the global proportion applied to the shortest incident edge.
- Worked example supplied by the owner: edges of length `10`, `7` and `5` meeting at one vertex give a local radius of `5 * 0.1 = 0.5`. The same rule is applied at every vertex, so a vertex where three long edges meet rounds further than a vertex pinched by one short edge.

### Per-Edge Interpolation

- Two vertices connected by an edge usually clamp to different radii, because their other incident edges differ in length.
- Along the edge, the effective radius varies linearly between the two clamped values. This is what keeps a complex figure continuous instead of stepping at every face.

## Open Questions

- How adjacency reaches the shader. Options are per-vertex attributes baked by the CPU (shortest incident edge length, or the clamped radius directly), or a topology buffer the shader indexes. Baking the clamped radius per vertex removes all lookups from the shader and is the natural continuation of the flat-payload rule of [Agnostic Pipeline](agnostic-pipeline.md), but it must be proven to keep the radius continuous across the face.
- How a fragment knows its triangle. The current renderer has no mesh path at all: it evaluates analytic figures from a ray. A triangle path needs barycentric evaluation of the interpolated radius, and the vertex radii are interpolated across the face, not the edge alone.
- How the dihedral angle is handled. Two faces meeting at a convex crease round outward; at a concave crease they round inward, which is the inversion of [Rounded Primitives](rounded-primitives.md) applied per crease. The rule must read the sign of the crease without branching per axis.
- Whether rounding is driven by edge length only, or by edge length and crease angle together. Edge length alone can round a crease that is nearly flat, which would bend a surface that should stay flat.

## Exclusions

- There is no mesh ingest format, no index buffer, and no triangle intersection path in the current code. The stage defines the rounding rule, not the data pipeline.
- Vertex normals and UV coordinates are not needed and are not part of the design, since shading is derived from inclination and color is flat per polygon.

## Validation

- The acceptance demonstration is a 3D arrow model rendered with the adaptive mesh. An arrow is chosen because it concentrates very different edge lengths in one figure: a long shaft, medium barbs, and short tip edges, so the proportional clamp is visible on a single object.

## Relations

- Requires the corner rule and the inversion of [Rounded Primitives](rounded-primitives.md).
- Supplies the geometry that [Orientation](orientation.md) rotates and that [Spatial Graph](spatial-graph.md) indexes spatially.
- Supplies the mesh path used by [Legacy Game Preservation](pickup-express.md), whose extracted models are the real-world test case for this rule.
