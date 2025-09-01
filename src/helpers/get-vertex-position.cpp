#include "get-vertex-position.hpp"
#include "../enums/lump.hpp"
#include "../errors.hpp"
#include <cmath>
#include <format>

namespace BspParser::Internal {
  const Structs::Vector& getVertexPosition(
    const std::span<const Structs::Edge> edges,
    const std::span<const Structs::Vector> vertices,
    const int32_t surfaceEdge
  ) {
    const auto edgeIndex = std::abs(surfaceEdge);
    if (edgeIndex >= edges.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::SurfaceEdges, std::format("Surface edge index '{}' is out of bounds of the edges lump", edgeIndex)
      );
    }

    const auto& edge = edges[edgeIndex];
    const auto firstVertexIndex = surfaceEdge < 0 ? edge.vertices.back() : edge.vertices.front();

    if (firstVertexIndex >= vertices.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::Edges,
        std::format("Edge vertex index '{}' is out of bounds of the vertices lump", firstVertexIndex)
      );
    }

    return vertices[firstVertexIndex];
  }
}
