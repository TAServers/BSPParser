#include "displacement.hpp"
#include "../../helpers/vector-maths.hpp"

namespace BspParser::Accessors::Internal {
  namespace {
    std::vector<Displacement> getEdgeNeighbours(const Bsp& bsp, const Structs::DispNeighbour& neighbour) {
      std::vector<Displacement> result;

      for (const auto& subNeighbour : neighbour.subNeighbors) {
        if (!subNeighbour.isValid()) {
          continue;
        }

        const auto& dispInfo = bsp.displacementInfos[subNeighbour.index];
        const auto& face = bsp.faces[dispInfo.mapFace];
        const auto& textureInfo = bsp.textureInfos[face.texInfo];
        const auto& textureData = bsp.textureDatas[textureInfo.texData];
        const auto& surfaceEdges = bsp.surfaceEdges.subspan(face.firstEdge, face.numEdges);

        result.emplace_back(bsp, dispInfo, textureInfo, textureData, surfaceEdges);
      }

      return std::move(result);
    }

    std::vector<Displacement> getCornerNeighbours(const Bsp& bsp, const Structs::DispCornerNeighbours& neighbours) {
      std::vector<Displacement> result;

      for (uint8_t i = 0; i < neighbours.numNeighbours; i++) {
        const auto& neighbour = neighbours.neighbours.at(i);

        const auto& dispInfo = bsp.displacementInfos[neighbour];
        const auto& face = bsp.faces[dispInfo.mapFace];
        const auto& textureInfo = bsp.textureInfos[face.texInfo];
        const auto& textureData = bsp.textureDatas[textureInfo.texData];
        const auto& surfaceEdges = bsp.surfaceEdges.subspan(face.firstEdge, face.numEdges);

        result.emplace_back(bsp, dispInfo, textureInfo, textureData, surfaceEdges);
      }

      return std::move(result);
    }
  }

  DisplacementWithNeighbours::DisplacementWithNeighbours(
    const Bsp& bsp,
    const Structs::DispInfo& dispInfo,
    const Structs::TexInfo& textureInfo,
    const Structs::TexData& textureData,
    const std::span<const int32_t> surfaceEdges
  ) : Displacement(bsp, dispInfo, textureInfo, textureData, surfaceEdges) {
    for (uint8_t neighbourIndex = 0; neighbourIndex < 4; neighbourIndex++) {
      edgeNeighbours[neighbourIndex] = getEdgeNeighbours(bsp, dispInfo.edgeNeighbours[neighbourIndex]);
      cornerNeighbours[neighbourIndex] = getCornerNeighbours(bsp, dispInfo.cornerNeighbours[neighbourIndex]);
    }
  }

  Structs::Vector DisplacementWithNeighbours::calculateNormal(const size_t x, const size_t y) const {
    const auto centre = calculatePosition(x, y);
    const auto leftEdgeEndPoint = findLeftEdgeVertex(x, y);
    const auto rightEdgeEndPoint = findRightEdgeVertex(x, y);
    const auto topEdgeEndPoint = findTopEdgeVertex(x, y);
    const auto bottomEdgeEndPoint = findBottomEdgeVertex(x, y);

    auto normal = Structs::Vector{};

    if (topEdgeEndPoint.has_value()) {
      if (leftEdgeEndPoint.has_value()) {
        normal = add(normal, cross(sub(topEdgeEndPoint.value(), centre), sub(leftEdgeEndPoint.value(), centre)));
      }

      if (rightEdgeEndPoint.has_value()) {
        normal = add(normal, cross(sub(rightEdgeEndPoint.value(), centre), sub(topEdgeEndPoint.value(), centre)));
      }
    }

    if (bottomEdgeEndPoint.has_value()) {
      if (leftEdgeEndPoint.has_value()) {
        normal = add(normal, cross(sub(leftEdgeEndPoint.value(), centre), sub(bottomEdgeEndPoint.value(), centre)));
      }

      if (rightEdgeEndPoint.has_value()) {
        normal = add(normal, cross(sub(bottomEdgeEndPoint.value(), centre), sub(rightEdgeEndPoint.value(), centre)));
      }
    }

    return normalise(normal);
  }

  std::optional<Structs::Vector> DisplacementWithNeighbours::findLeftEdgeVertex(const size_t x, const size_t y) const {
    if (x > 0) {
      return calculatePosition(x - 1, y);
    }

    return std::nullopt;
  }

  std::optional<Structs::Vector> DisplacementWithNeighbours::findRightEdgeVertex(const size_t x, const size_t y) const {
    if (x < getNumVerticesPerAxis() - 1) {
      return calculatePosition(x + 1, y);
    }

    return std::nullopt;
  }

  std::optional<Structs::Vector> DisplacementWithNeighbours::findTopEdgeVertex(const size_t x, const size_t y) const {
    if (y < getNumVerticesPerAxis() - 1) {
      return calculatePosition(x, y + 1);
    }

    return std::nullopt;
  }

  std::optional<Structs::Vector> DisplacementWithNeighbours::findBottomEdgeVertex(
    const size_t x, const size_t y
  ) const {
    if (y > 0) {
      return calculatePosition(x, y - 1);
    }

    return std::nullopt;
  }
}
