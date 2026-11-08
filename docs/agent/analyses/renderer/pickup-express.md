# Legacy Game Preservation

Import: [3D Renderer](index.md)

## Scope

- Stage 7 and the ethical frame of the whole domain: use the meshes of a legacy game as the real-world validation of the rounded-surface renderer.
- Records why the source material is legitimate, what the first visual target is, and the visual thesis the domain exists to test.

## Facts

- The source project is the sibling repository `pickup-express`, reachable from this repository root as `../pickup-express`. The path was confirmed to exist in this session.
- The repository contains reverse-engineered meshes of the game objects, extracted for preservation.
- Evidence: the sibling directory listing; the ownership statement below.

## Ethical Scope

The owner considers the work legitimate on three grounds, each of which is necessary:

- The game has no commercial relevance.
- The game was distributed for free at its own launch.
- The developing company and the publishing company both ceased to exist many years ago.

- Consequence for the domain: the source material is used as an input corpus for a rendering experiment. Preservation of the game is the purpose of the sibling project, not of this one.

## Decisions

### First Visual Target

- No textures in the first delivery.
- The CPU samples each polygon's original texture and reduces it to the dominant average color for that polygon.
- Each polygon is filled with that single flat color.
- The rounded-edge geometry and the inclination brightness are applied over the flat color.

### Visual Thesis

- A flat color shaded by inclination, on analytically rounded edges, is more pleasant to look at than the original texture on sharp polygons.
- The broader hypothesis, explicitly held by the owner and still to be tested: the same holds for modern games, whose visual coherence would improve by prioritising geometric perfection over photographic texturisation.

## Open Questions

- How the dominant color per polygon is defined: mean in linear space, mode of a quantised histogram, or the color at the polygon centroid. Each gives a different flat color for the same texture.
- Whether a polygon's flat color survives the rounding transition. The rounding blends two faces near an edge, and the blend of two flat colors must be decided rather than left implicit.
- Which model is used first. An arrow is reserved for [Adaptive Mesh](adaptive-mesh.md); the game corpus should contribute a model whose edge lengths vary enough to show the proportional clamp.

## Exclusions

- The stage does not read, parse, or convert the game's asset formats. The sibling project already owns that; this domain consumes its meshes.
- No texture filtering, no UV mapping, and no sampling in the shader are part of the delivery.
- No measurement of the game corpus (model count, polygon count) is recorded here; those numbers belong to the sibling project.

## Relations

- Requires the mesh path of [Adaptive Mesh](adaptive-mesh.md) and the visible-set discovery that [Spatial Graph](spatial-graph.md) provides for scenes far larger than a two-object prototype.
- Uses the inclination brightness of [Baseline Reference](baseline.md) unchanged, which is the point: the shading model proven on two spheres must survive contact with real models.
