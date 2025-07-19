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

    const auto numVerticesPerAxis = (1ul << static_cast<size_t>(dispInfo.power)) + 1;
    const auto edgeLengthFraction = 1.f / static_cast<float>(numVerticesPerAxis - 1);

    const auto edgeIncrements = std::array{
      mul(sub(corners[1], corners[0]), edgeLengthFraction),
      mul(sub(corners[2], corners[3]), edgeLengthFraction),
    };

    for (size_t i = 0; i < numVerticesPerAxis; i++) {
      const auto endPoints = std::array{
        add(mul(edgeIncrements[0], static_cast<float>(i)), corners[0]),
        add(mul(edgeIncrements[1], static_cast<float>(i)), corners[3]),
      };

      const auto segment = sub(endPoints[1], endPoints[0]);
      const auto segmentIncrement = mul(segment, edgeLengthFraction);

      for (size_t j = 0; j < numVerticesPerAxis; j++) {
        const auto vertexIndex = i * numVerticesPerAxis + j;
        const auto& displacementVertex = bsp.displacementVertices[dispInfo.dispVertStart + vertexIndex];

        const auto segmentOffset = mul(segmentIncrement, static_cast<float>(j));
        const auto displacementVector = mul(displacementVertex.vec, displacementVertex.dist);

        iteratee(
          Vertex{
            .position = add(add(endPoints[0], segmentOffset), displacementVector),
            .alpha = std::clamp(displacementVertex.alpha / 255.f, 0.f, 1.f),
          }
        );
      }
    }
  }
}
