# BSPParser

Simple and modern library for parsing the Valve BSP map format.

Documentation: https://taservers.github.io/BSPParser/

See also:

- https://github.com/TAServers/MDLParser
- https://github.com/TAServers/VTFParser
- https://github.com/TAServers/PHYParser

## What's included

- Class for parsing and abstracing the `BSP` file format for the Source engine.
- Helper functions to simplify accessing the complex data structures of the BSP (see example below)
- Enums and structs with decent coverage of the BSP lumps and their versions.
- Runtime errors for issues when parsing the data due to corruption or a bug in the parser.

## Example

Generating a triangle list for all models in the BSP:

```cpp
#include "BSPParser.hpp"

// Load from a file on disk, an API, or somewhere in memory
const auto bspData = ...;

// Parse the data (you should wrap this in a try/catch)
const BspParser::Bsp bsp(bspData);

// We want to render the BSP, so pay the performance cost of smoothing displacement normals
bsp.smoothNeighbouringDisplacements();

// For each model...
BspParser::Accessors::iterateModels(
  bsp,
  [&bsp](
    const BspParser::Structs::Model& model,
    const std::vector<BspParser::PhysModel>& physicsModels
  ) {
    // For each face...
    BspParser::Accessors::iterateFaces(
      bsp,
      model,
      [&bsp](
        const BspParser::Structs::Face& face,
        const BspParser::Structs::Plane& plane,
        const BspParser::Structs::TexInfo& textureInfo,
        const std::span<const int32_t> surfaceEdges
      ) {
        if (isFaceNoDraw(surfaceEdges, textureInfo)) {
          return;
        }

        // For each vertex... (you can use getVertexCount to preallocate memory for a single face or the whole BSP)
        BspParser::Accessors::generateVertices(
          bsp,
          face,
          plane,
          textureInfo,
          surfaceEdges,
          [](const BspParser::Vertex& vertex) {
            // Use vertex.position, vertex.normal, etc.
          }
        );

        // For each index... (you can use getTriangleListIndexCount to preallocate memory here too)
        BspParser::Accessors::generateTriangleListIndices(
          bsp,
          face,
          surfaceEdges,
          [](const int32_t index0, const int32_t index1, const int32_t index2) {
            // Use triangle indices (always clockwise winding)
          }
        );
      }
    );
  }
);
```

Generating colliders for all physmeshes in the BSP:

```cpp
#include "BSPParser.hpp"

// Load from a file on disk, an API, or somewhere in memory
const auto bspData = ...;

// Parse the data (you should wrap this in a try/catch)
const BspParser::Bsp bsp(bspData);

// We don't want to render the BSP, so don't smooth normals this time
// bsp.smoothNeighbouringDisplacements();

// For each model...
BspParser::Accessors::iterateModels(
  bsp,
  [&bsp](
    const BspParser::Structs::Model& model,
    const std::vector<BspParser::PhysModel>& physicsModels
  ) {
    for (const auto& physicsModel : physicsModels) {
      // You don't have to use PhyParser here, any solution for parsing .phy file data will do
      const auto [solidsForModel, _] = PhyParser::parseSurfaces(physicsModel.collisionData, physicsModel.solidCount);

      // Use solids to build your colliders...
    }
  }
);
```
