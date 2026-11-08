# Spatial Graph

Import: [3D Renderer](index.md)

## Scope

- Stage 6: replace the evaluation of every object in the buffer with the evaluation of the objects the camera can actually see, discovered incrementally through a doubly-linked spatial graph.
- The graph is a CPU structure. The GPU keeps evaluating from a buffer; the stage changes how that buffer is filled and bounded.

## Decisions

### Topology

- Every world object is a node connected to its immediate spatial neighbours through doubly-linked pointers.
- When an object moves, it walks its existing links to find its new neighbourhood and rewires locally. There is no global rebuild of a spatial tree.

### Camera as a Node

- The camera is a node of the same graph and is not rendered.
- The camera holds two boundary pointers: the farthest visible object at the upper left of the view, and the farthest visible object at the lower left.
- When the camera moves, it walks from its own node and from those two boundary pointers to find the new boundary objects, instead of rescanning the world.

### Frame Feed

- Before a frame is rendered, the CPU traverses the region bounded by the camera node and the two boundary objects to collect the object indices to evaluate.
- The GPU loop is bounded by that selection rather than by the whole world.

## Open Questions

- How a neighbour is defined in three dimensions in a way that stays symmetric. Fixed directional slots per axis tie the structure to a dimension count; a distance-based insertion keeps it dimensional but costs more on insert.
- Whether two boundary pointers bound a frustum at all. Two corners of a frustum do not define a region in three dimensions; the stage must either add the remaining extremes or accept that the traversal is an approximation and record that it is one.
- How the selection reaches a shader whose loop is driven by the buffer header count. Options are an index buffer that the shader reads to fetch each object index, or a compacted object buffer rewritten per frame. The first avoids copying payloads; the second keeps the shader free of indirection.
- What the graph does when the view points at empty space. A boundary pointer needs a sentinel meaning no object in that direction, and the traversal must terminate on it.
- Whether the camera node also needs vertical extremes. The two pointers given are both on the left side of the view; the top and bottom edges of the frustum are covered only if the pair is chosen as opposites rather than as two left corners.

## Exclusions

- No graph, no traversal, and no index buffer exist. This stage states the intent, which the owner has explicitly marked as immature.
- Nothing in this stage changes the figure contract; it only changes which figures are submitted.
- Load balancing, level of detail, and frame-to-frame caching of the traversal are outside the stage.

## Relations

- Requires the object list produced by [Agnostic Pipeline](agnostic-pipeline.md) and the world-scale meshes of [Adaptive Mesh](adaptive-mesh.md).
- Independent of [Orientation](orientation.md) in implementation, but both write into the same per-object payload, so the payload slot layout is shared.
- Is the mechanism that makes [Legacy Game Preservation](pickup-express.md) viable at real scene sizes, where the extracted models are many and small.
