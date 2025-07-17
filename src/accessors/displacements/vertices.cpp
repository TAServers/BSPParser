#include "vertices.hpp"
#include "../../helpers/vector-maths.hpp"
#include "../get-vertex-position.hpp"

namespace BspParser::Accessors::Internal {
  namespace {
    std::array<Structs::Vector, 4> getCorners(
      const Bsp& bsp, const Structs::DispInfo& dispInfo, const std::span<const int32_t> surfaceEdges
    ) {
      std::array<Structs::Vector, 4> corners;
      size_t firstCorner = 0;
      auto firstCornerDistanceSquared = std::numeric_limits<float>::max();

      for (size_t surfEdgeIndex = 0; surfEdgeIndex < std::min(surfaceEdges.size(), 4ul); surfEdgeIndex++) {
        const auto vertex = getVertexPosition(bsp, surfaceEdges[surfEdgeIndex]);
        corners.at(surfEdgeIndex) = vertex;

        const auto displacementVector = sub(dispInfo.startPosition, vertex);
        const auto distanceSquared = dot(displacementVector, displacementVector);

        if (distanceSquared < firstCornerDistanceSquared) {
          firstCorner = surfEdgeIndex;
          firstCornerDistanceSquared = distanceSquared;
        }
      }

      std::array<Structs::Vector, 4> remapped;
      for (size_t i = 0; i < 4; i++) {
        remapped.at(i) = corners.at((i + firstCorner) % 4);
      }

      return remapped;
    }
  }

  void generateDisplacementVertices(
    const Bsp& bsp,
    const Structs::DispInfo& dispInfo,
    const Structs::TexInfo& textureInfo,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  ) {
    const auto corners = getCorners(bsp, dispInfo, surfaceEdges);
  }
}
