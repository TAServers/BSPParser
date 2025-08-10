#include "triangulated-displacement.hpp"
#include "../helpers/calculate-uvs.hpp"
#include "../helpers/get-vertex-position.hpp"
#include "../helpers/vector-maths.hpp"
#include <algorithm>

namespace BspParser {
  using namespace Internal;

  namespace {
    std::array<Structs::Vector, 4> getCorners(
      const Structs::DispInfo& dispInfo,
      const std::span<const Structs::Edge> edges,
      const std::span<const Structs::Vector> vertices,
      const std::span<const int32_t> surfaceEdges
    ) {
      std::array<Structs::Vector, 4> corners;
      size_t firstCorner = 0;
      auto firstCornerDistanceSquared = std::numeric_limits<float>::max();

      for (size_t surfEdgeIndex = 0; surfEdgeIndex < std::min(surfaceEdges.size(), 4ul); surfEdgeIndex++) {
        const auto vertex = getVertexPosition(edges, vertices, surfaceEdges[surfEdgeIndex]);
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

    Structs::Vector calculateTessellatedPosition(
      const Structs::DispVert& displacementVertex,
      const float edgeLengthFraction,
      const std::array<Structs::Vector, 4>& corners,
      const std::array<Structs::Vector, 2>& increments,
      const size_t x,
      const size_t y
    ) {
      const auto endPoints = std::array{
        add(mul(increments[0], static_cast<float>(y)), corners[0]),
        add(mul(increments[1], static_cast<float>(y)), corners[3]),
      };

      const auto segment = sub(endPoints[1], endPoints[0]);
      const auto segmentIncrement = mul(segment, edgeLengthFraction);
      const auto segmentOffset = mul(segmentIncrement, static_cast<float>(x));

      const auto displacementVector = mul(displacementVertex.vec, displacementVertex.dist);

      return add(add(endPoints[0], segmentOffset), displacementVector);
    }

    Structs::Vector2 calculateTessellatedUv(
      const float edgeLengthFraction,
      const std::array<Structs::Vector2, 4>& corners,
      const std::array<Structs::Vector2, 2>& increments,
      const size_t x,
      const size_t y
    ) {
      const auto endPoints = std::array{
        add(mul(increments[0], static_cast<float>(y)), corners[0]),
        add(mul(increments[1], static_cast<float>(y)), corners[3]),
      };

      const auto segment = sub(endPoints[1], endPoints[0]);
      const auto segmentIncrement = mul(segment, edgeLengthFraction);
      const auto segmentOffset = mul(segmentIncrement, static_cast<float>(x));

      return add(endPoints[0], segmentOffset);
    }
  }

  std::vector<Vertex> TriangulatedDisplacement::triangulate(
    const std::span<const Structs::DispVert> dispVertices,
    const std::span<const Structs::Edge> edges,
    const std::span<const Structs::Vector> vertices,
    const std::span<const int32_t> surfaceEdges
  ) const {
    const auto edgeLengthFraction = 1.f / static_cast<float>(numVerticesPerAxis - 1);

    const auto dispVerticesForDisplacement =
      dispVertices.subspan(dispInfo.dispVertStart, numVerticesPerAxis * numVerticesPerAxis);

    const auto cornerPositions = getCorners(dispInfo, edges, vertices, surfaceEdges);
    const auto cornerUvs = std::array{
      calculateUvs(cornerPositions[0], textureInfo, textureData),
      calculateUvs(cornerPositions[1], textureInfo, textureData),
      calculateUvs(cornerPositions[2], textureInfo, textureData),
      calculateUvs(cornerPositions[3], textureInfo, textureData),
    };

    const auto positionIncrements = std::array{
      mul(sub(cornerPositions[1], cornerPositions[0]), edgeLengthFraction),
      mul(sub(cornerPositions[2], cornerPositions[3]), edgeLengthFraction),
    };
    const auto uvIncrements = std::array{
      mul(sub(cornerUvs[1], cornerUvs[0]), edgeLengthFraction),
      mul(sub(cornerUvs[2], cornerUvs[3]), edgeLengthFraction),
    };

    std::vector<Vertex> triangulatedVertices;
    triangulatedVertices.reserve(numVerticesPerAxis * numVerticesPerAxis);

    for (size_t y = 0; y < numVerticesPerAxis; y++) {
      for (size_t x = 0; x < numVerticesPerAxis; x++) {
        const auto& displacementVertex = dispVerticesForDisplacement[y * numVerticesPerAxis + x];

        triangulatedVertices.push_back(
          Vertex{
            .position = calculateTessellatedPosition(
              displacementVertex, edgeLengthFraction, cornerPositions, positionIncrements, x, y
            ),
            .normal = Structs::Vector{},
            .tangent = Structs::Vector4{},
            .uv = calculateTessellatedUv(edgeLengthFraction, cornerUvs, uvIncrements, x, y),
            .alpha = std::clamp(displacementVertex.alpha / 255.f, 0.f, 1.f),
          }
        );
      }
    }

    return std::move(triangulatedVertices);
  }
}
