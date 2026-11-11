# Legacy Game Preservation

## Scope

- Real-world validation of the analytical rounded-surface renderer using preserved legacy game meshes from sibling repository `../pickup-express`, shaded with inclination brightness over one flat dominant color per polygon.
- Establishes the domain's preserved-asset integration architecture:
  - Ethical preservation grounds and repository boundary with `../pickup-express`.
  - Visual coherence thesis comparing analytical rounded geometry against low-resolution photographic texturing.
  - Verified `.ccf` and `.gti` data structures separated from premeditated renderer integration rules.

## Ethical Preservation Scope

- Legitimacy of using the legacy source material rests on the following necessary grounds established by the owner:
  - Zero commercial relevance of the original title.
  - Free distribution at its original launch.
  - Long-standing cessation of operations by both the developer and the publisher.
- Repository role separation:
  - Sibling repository `../pickup-express` owns:
    - Binary format reverse engineering.
    - `.ccf` and `.gti` asset parsers.
    - Archive extraction and game preservation.
  - `draft-point` consumes the decoded geometry and textures strictly as an experimental rendering corpus.

## Visual Thesis

- Analytical edge and corner rounding paired with ray-surface inclination brightness over flat per-polygon colors achieves higher visual coherence and pleasantness than sharp low-polygon geometry wrapped in low-resolution photographic textures.
- Broader hypothesis under investigation: real-time 3D graphics likewise gain stronger visual coherence by prioritizing continuous geometric curvature over photographic surface texturing.

## Verified Sibling Corpus Architecture

- **Coordinate Convention**:
  - Decoded geometry in `../pickup-express` inhabits a right-handed metric coordinate system:
    - `+X` points right.
    - `+Y` points up.
    - `-Z` points forward.
  - Front-facing triangles use counter-clockwise winding with outward geometric normal `cross( second - first, third - first )`.
- **`.ccf` Chunk Distinction (`0x3100` vs `0x1100`)**:
  - **Chunk `0x3100` (`CcMesh` visual object groups)** stores named visual submesh groups carrying:
    - Local node origin (`0xf040`).
    - Orthonormal $3 \times 3$ orientation basis (`0xf070` / `0xf050`).
    - Shared geometric vertex table (`0x3110`).
    - Indexed triangle face table (`0x3120`).
  - Each `0x3120` face record carries:
    - Material identifier referencing `0x2100`.
    - Per-face UV coordinates (`0xf060`).
    - Optional per-face flat RGB color (`0xf090`).
  - Storing UVs and materials on `0x3120` faces keeps `0x3110` vertices geometrically shared within each `0x3100` group across UV and material boundaries.
  - **Chunk `0x1100` (Town static collision BSP)** stores static collision geometry:
    - Unindexed BSP-clipped leaf polygons (`0xf0c1`) split across partition planes with T-junctions.
    - Collision surface identifiers disjoint from visual `0x2100` materials.
  - High-level helper `pickup::ccf::parsed` flattens `0x3120` face indices into unindexed triangles and omits per-group `0xf040` origins and `0xf070` orientation bases.
- **`.gti` Texture Subtypes**:
  - `pickup::gti::decode_base_rgba` (`richest_index`) decodes legacy `.gti` pixel subtypes to RGBA8888 buffers on the CPU:
    - `P8` paletted color.
    - `AP88` alpha-plus-palette color.
    - `BGR888` packed truecolor.
    - `RGBA8888` packed truecolor with alpha.

## Premeditated Renderer Integration

- **Per-Group Manifold Scoping**:
  - Visual mesh ingestion consumes `0x3100` visual groups exclusively while leaving `0x1100` collision BSP chunks inside `../pickup-express`.
  - Manifold edge adjacency for the adaptive `border_radius` baker of [Adaptive Mesh](adaptive-mesh.md) scopes strictly per `0x3100` submesh group by retaining `0x3110` vertex and `0x3120` face index tables (or applying exact bitwise per-group vertex deduplication).
- **Articulated Submesh Placement**:
  - Each `0x3100` group's `0xf040` translation maps into `m_position` and its `0xf070` orthonormal basis maps into the `orientation_basis` GPU payload slots of [Orientation](orientation.md), while `m_parameters` (`figure_item.parameters`) indexes the group's `triangle_ssbo` slice.
  - Preserving per-group local frames places articulated multi-group vehicles with wheel submeshes positioned at their proper hubs.
- **UV-Wrapped Modal Linear-RGB Color Reduction**:
  - Per-triangle flat color resolution runs on the CPU during mesh baking through a deterministic per-`0x3120` face fallback chain:
    - Textured faces carrying a `0x2100` material and `0xf060` UVs sample the decoded RGBA8888 texels covered by the UV triangle, applying `fract( uv )` wrap for tiled UVs outside `[ 0.0f, 1.0f ]`.
    - Untextured faces read their flat color directly from the `0xf090` face RGB record.
  - Dominant color reduction ("most frequent average color per polygon") isolates primary surface paint from thin UV-atlas trim and window frames:
    - Sampled texels convert to linear RGB and quantize into coarse histogram bins.
    - Reduction selects the modal (most populated) bin and averages the exact linear-RGB values falling inside that dominant bin.

## Progressive Validation Ladder

| Asset Path (`../pickup-express`) | Topological Profile | Validation Target |
|---|---|---|
| `Packages/box/box.ccf` | Single-group closed manifold box prop | Direct visual and geometric parity with the analytical rounded box of [Rounded Primitives](rounded-primitives.md) |
| `General/arrow.ccf` | Single-group closed manifold arrow prop | Real-asset parity with convex ridge and concave notch rounding of [Adaptive Mesh](adaptive-mesh.md) |
| `Vehicles/Cars/City.ccf` | Multi-group articulated vehicle with separate body and wheel submeshes | Per-group `0xf040` and `0xf070` placement through [Orientation](orientation.md) combined with UV-atlas modal flat colors |
| `Towns/Chinatown/chinatown-express.ccf` (`0x3100` groups) | Multi-object town environment with buildings and thin-sheet props | Incremental visibility culling via [Spatial Graph](spatial-graph.md) combined with open-boundary sheet handling from [Adaptive Mesh](adaptive-mesh.md) |

## Open Questions

| Topic | Option A | Option B | Trade-Off |
|---|---|---|---|
| **Cross-edge color transition across rounded fillets** | Discrete color boundary at the dihedral angle-bisector plane | Smooth fillet-weight color blend matching normal interpolation across the arc | Shared with [Adaptive Mesh](adaptive-mesh.md): Option A preserves crisp polygon paint regions; Option B softens high-contrast seams between differently colored adjacent faces |
| **Multi-face corner color weighting** | Dominant-weight selection from the nearest incident triangle face | Barycentric or fillet-weight blend across all faces meeting at the rounded corner | Follows directly from the cross-edge color transition choice at shared vertices |
| **Modal linear-RGB histogram granularity** | Fixed uniform channel quantization across linear RGB | Adaptive perceptual bin width separating dark atlas trim from body paint | Option A is deterministic and parameter-free; Option B adapts to low-contrast textures |
| **Sub-texel UV triangle sampling** | Centroid texel fallback when wrapped UV footprint falls below one texel pitch | Fixed supersampled barycentric grid across every UV triangle | Option A avoids redundant identical texel reads; Option B keeps one uniform sampling loop |
| **Alpha-cutout `.gti` policy (`AP88`, `RGBA8888`)** | Exclude transparent texels below an alpha threshold during modal color reduction while keeping the polygon opaque | Carry a binary cutout flag for perforated grate and foliage faces | Option A preserves opaque flat-shaded silhouettes; Option B preserves perforated prop silhouettes |

## Boundary

- Sibling repository `../pickup-express` owns:
  - Binary format reverse engineering.
  - `.ccf` and `.gti` parsers and archive extraction.
  - Corpus-wide asset statistics and game preservation documentation.
- Inclination brightness shading belongs to [Baseline Reference](baseline.md).
- Flat SSBO buffer transport belongs to [Agnostic Pipeline](agnostic-pipeline.md).
- Analytical convex and concave fillet mathematics belong to [Rounded Primitives](rounded-primitives.md).
- Per-vertex clamped `border_radius` baking and mesh crease evaluation belong to [Adaptive Mesh](adaptive-mesh.md).
- Per-figure orthonormal basis transforms belong to [Orientation](orientation.md).
- Incremental visible-set discovery belongs to [Spatial Graph](spatial-graph.md).
- This specification owns the ingestion adapter from decoded `../pickup-express` `0x3100` groups and `.gti` RGBA buffers into per-group manifold mesh payloads and CPU modal linear-RGB polygon colors.
- Residing outside this domain:
  - GPU texture residency.
  - Shader UV interpolation.
  - Runtime texture filtering.

## Relations

- Parent domain: [3D Renderer](index.md)
- Upstream inclination brightness law: [Baseline Reference](baseline.md)
- Upstream flat SSBO payload contract: [Agnostic Pipeline](agnostic-pipeline.md)
- Upstream adaptive mesh baker and crease evaluator: [Adaptive Mesh](adaptive-mesh.md)
- Upstream per-group orientation transform contract: [Orientation](orientation.md)
- Upstream multi-object visibility culling feed: [Spatial Graph](spatial-graph.md)
