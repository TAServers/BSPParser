#include "triangulated-displacement.hpp"

namespace BspParser {
  namespace {
    std::vector<uint16_t> getEdgeNeighbours(const Structs::DispNeighbour& neighbour) {
      std::vector<uint16_t> result;

      for (const auto& subNeighbour : neighbour.subNeighbors) {
        if (!subNeighbour.isValid()) {
          continue;
        }

        result.push_back(subNeighbour.index);
      }

      return std::move(result);
    }

    std::vector<uint16_t> getCornerNeighbours(const Structs::DispCornerNeighbours& neighbours) {
      std::vector<uint16_t> result;

      for (uint8_t i = 0; i < neighbours.numNeighbours; i++) {
        const auto& neighbour = neighbours.neighbours.at(i);

        result.push_back(neighbour);
      }

      return std::move(result);
    }
  }

  TriangulatedDisplacement::TriangulatedDisplacement(
    const Structs::DispInfo& dispInfo,
    const std::span<const Structs::DispVert> dispVertices,
    const std::span<const Structs::Edge> edges,
    const std::span<const Structs::Vector> vertices,
    const std::span<const int32_t> surfaceEdges,
    const Structs::TexInfo& textureInfo,
    const Structs::TexData& textureData
  ) : dispInfo(dispInfo), textureInfo(textureInfo), textureData(textureData) {
    numVerticesPerAxis = (1ul << static_cast<size_t>(dispInfo.power)) + 1;

    for (uint8_t neighbourIndex = 0; neighbourIndex < 4; neighbourIndex++) {
      edgeNeighbours[neighbourIndex] = getEdgeNeighbours(dispInfo.edgeNeighbours[neighbourIndex]);
      cornerNeighbours[neighbourIndex] = getCornerNeighbours(dispInfo.cornerNeighbours[neighbourIndex]);
    }

    this->vertices = triangulate(dispVertices, edges, vertices, surfaceEdges);
    generateInternalNormals();
  }

  size_t TriangulatedDisplacement::getTriangleListIndexCount() const {
    constexpr auto numVerticesPerTriangle = 3;
    constexpr auto numTrianglesPerQuad = 2;

    const auto numEdgesPerAxis = numVerticesPerAxis - 1;

    return numEdgesPerAxis * numEdgesPerAxis * numTrianglesPerQuad * numVerticesPerTriangle;
  }

  void TriangulatedDisplacement::generateTriangleListIndices(
    const std::function<void(uint32_t i0, uint32_t i1, uint32_t i2)>& iteratee
  ) const {
    const auto size = numVerticesPerAxis - 1;

    for (uint32_t x = 0; x < size; x++) {
      for (uint32_t y = 0; y < size; y++) {
        const auto bottomLeft = getVertexIndex(x, y);
        const auto topLeft = getVertexIndex(x, y + 1);
        const auto topRight = getVertexIndex(x + 1, y + 1);
        const auto bottomRight = getVertexIndex(x + 1, y);

        iteratee(bottomLeft, topLeft, topRight);
        iteratee(bottomLeft, topRight, bottomRight);
      }
    }
  }

  size_t TriangulatedDisplacement::getVertexIndex(const size_t x, const size_t y) const {
    return y * numVerticesPerAxis + x;
  }

  const Vertex& TriangulatedDisplacement::getVertex(const size_t x, const size_t y) const {
    return vertices[getVertexIndex(x, y)];
  }
}
