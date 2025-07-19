#include "displacement.hpp"
#include "../../helpers/vector-maths.hpp"
#include "../calculate-uvs.hpp"
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

  Displacement::Displacement(
    const Bsp& bsp,
    const Structs::DispInfo& dispInfo,
    const Structs::TexInfo& textureInfo,
    const Structs::TexData& textureData,
    const std::span<const int32_t> surfaceEdges
  ) {
    numVerticesPerAxis = (1ul << static_cast<size_t>(dispInfo.power)) + 1;
    edgeLengthFraction = 1.f / static_cast<float>(numVerticesPerAxis - 1);

    vertices = bsp.displacementVertices.subspan(dispInfo.dispVertStart, numVerticesPerAxis * numVerticesPerAxis);

    cornerPositions = getCorners(bsp, dispInfo, surfaceEdges);

    cornerUvs = std::array{
      calculateUvs(cornerPositions[0], textureInfo, textureData),
      calculateUvs(cornerPositions[1], textureInfo, textureData),
      calculateUvs(cornerPositions[2], textureInfo, textureData),
      calculateUvs(cornerPositions[3], textureInfo, textureData),
    };

    positionIncrements = std::array{
      mul(sub(cornerPositions[1], cornerPositions[0]), edgeLengthFraction),
      mul(sub(cornerPositions[2], cornerPositions[3]), edgeLengthFraction),
    };
    uvIncrements = std::array{
      mul(sub(cornerUvs[1], cornerUvs[0]), edgeLengthFraction),
      mul(sub(cornerUvs[2], cornerUvs[3]), edgeLengthFraction),
    };
  }

  size_t Displacement::getNumVerticesPerAxis() const {
    return numVerticesPerAxis;
  }

  Structs::Vector Displacement::calculatePosition(const size_t x, const size_t y) const {
    const auto endPoints = std::array{
      add(mul(positionIncrements[0], static_cast<float>(y)), cornerPositions[0]),
      add(mul(positionIncrements[1], static_cast<float>(y)), cornerPositions[3]),
    };

    const auto segment = sub(endPoints[1], endPoints[0]);
    const auto segmentIncrement = mul(segment, edgeLengthFraction);
    const auto segmentOffset = mul(segmentIncrement, static_cast<float>(x));

    const auto& displacementVertex = vertices[y * numVerticesPerAxis + x];
    const auto displacementVector = mul(displacementVertex.vec, displacementVertex.dist);

    return add(add(endPoints[0], segmentOffset), displacementVector);
  }

  Structs::Vector2 Displacement::calculateUv(const size_t x, const size_t y) const {
    const auto endPoints = std::array{
      add(mul(uvIncrements[0], static_cast<float>(y)), cornerUvs[0]),
      add(mul(uvIncrements[1], static_cast<float>(y)), cornerUvs[3]),
    };

    const auto segment = sub(endPoints[1], endPoints[0]);
    const auto segmentIncrement = mul(segment, edgeLengthFraction);
    const auto segmentOffset = mul(segmentIncrement, static_cast<float>(x));

    return add(endPoints[0], segmentOffset);
  }

  float Displacement::getAlpha(const size_t x, const size_t y) const {
    const auto& displacementVertex = vertices[y * numVerticesPerAxis + x];

    return std::clamp(displacementVertex.alpha / 255.f, 0.f, 1.f);
  }
}
