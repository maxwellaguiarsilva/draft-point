# Rounded Primitives

Import: [3D Renderer](index.md)

## Scope

- Stage 3: a hybrid figure that is a cube whose near-edge region is evaluated as a sphere, so the edges render round without extra geometry.
- Stage 3b: the same hybrid figure cut open, so the rounded surface is visible from the inside as a concave fillet.
- This is the smallest stage that demonstrates the whole thesis of the domain: a sharp primitive becomes perfectly round analytically, with no tessellation.

## Decisions

### Hybrid Rounded Cube

- The figure carries a `float border_radius` interpreted as a proportion of the cube size. `0.1f` means the rounding reaches ten percent of the cube inward from its edges.
- Inside the core region, the surface is evaluated with the cube formula: flat faces, flat edges, flat corners.
- Inside the border region, the surface is evaluated with the sphere formula anchored at the inset point, so the transition produces a fillet on edges and a spherical octant on corners.
- The border region is selected by the same normalized measure that the sphere already uses: how far the hit point sits from the region boundary. The threshold is a comparison on that measure, not a branch on individual axes.

### Interior Inversion

- Cutting the hybrid cube in half exposes the inner walls and the inner side of the rounding.
- The requirement is that the interior is a genuine concave fillet: the surface normals of the inner corner must curve continuously from one inner wall into the other, mirroring the convex case.
- The inversion is not a sign flip on the final brightness. It changes which side of the surface is evaluated against the ray, so the transition point between the flat wall and the fillet must be found on the inside of the figure.

## Open Questions

- Whether the hybrid is one figure with a parameter or two figures registered separately. One figure keeps the transition in a single body; two figures keep each body readable and let the dispatch select the shape.
- The exact measure that separates core from border. The sphere uses the squared distance from the ray to the center normalized by the radius; the cube needs the equivalent normalized measure against its inset edges, and it must not be written per axis.
- Whether the cut is a half-space test applied before the figure evaluation, or a second figure type whose payload is the cut plane. The first keeps the figure count low; the second keeps the figure body free of scene composition.
- How the inner surface reports its distance for the outer distance comparison, given that the ray now enters through the cut face and exits through the inner fillet.

## Exclusions

- No blend function between cube and sphere is defined; the transition rule is the central undecided point of the stage.
- No shell thickness is defined. The stage renders the inner surface of a solid cut, not a wall of measurable thickness.
- Nothing is implemented, compiled, or measured yet.

## Relations

- Requires the dispatch of [Agnostic Pipeline](agnostic-pipeline.md): without injected figure bodies there is no place for a hybrid to live.
- The corner-rounding rule for a vertex with three incident edges is generalised in [Adaptive Mesh](adaptive-mesh.md); this stage fixes the rule for one primitive before it is applied to arbitrary topology.
- Uses the inclination brightness defined in [Baseline Reference](baseline.md) unchanged.
