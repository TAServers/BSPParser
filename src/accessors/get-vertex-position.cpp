#include "get-vertex-position.hpp"

namespace BspParser::Accessors::Internal {
  const Structs::Vector& getVertexPosition(const Bsp& bsp, const int32_t surfaceEdge) {
    const auto edgeIndex = std::abs(surfaceEdge);
    if (edgeIndex >= bsp.edges.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::SurfaceEdges, std::format("Surface edge index '{}' is out of bounds of the edges lump", edgeIndex)
      );
    }

    const auto& edge = bsp.edges[edgeIndex];
    const auto firstVertexIndex = surfaceEdge < 0 ? edge.vertices.back() : edge.vertices.front();

    if (firstVertexIndex >= bsp.vertices.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::Edges,
        std::format("Edge vertex index '{}' is out of bounds of the vertices lump", firstVertexIndex)
      );
    }

    return bsp.vertices[firstVertexIndex];
  }
}
