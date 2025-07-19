#include "displacement.hpp"

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

  DisplacementNormals::DisplacementNormals(const Bsp& bsp, const Structs::DispInfo& dispInfo) {
    for (uint8_t neighbourIndex = 0; neighbourIndex < 4; neighbourIndex++) {
      edgeNeighbours[neighbourIndex] = getEdgeNeighbours(bsp, dispInfo.edgeNeighbours[neighbourIndex]);
      cornerNeighbours[neighbourIndex] = getCornerNeighbours(bsp, dispInfo.cornerNeighbours[neighbourIndex]);
    }
  }

  Structs::Vector DisplacementNormals::calculateNormal(const size_t x, const size_t y) const {
    return Structs::Vector{.x = 0, .y = 0, .z = 1};
  }
}
