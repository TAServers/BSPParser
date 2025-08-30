#pragma once

#include "../structs/displacements.hpp"
#include "../structs/geometry.hpp"
#include "../structs/textures.hpp"
#include "../vertex.hpp"
#include <functional>
#include <span>
#include <vector>

namespace BspParser {
  class TriangulatedDisplacement {
  public:
    static constexpr uint8_t CORNER_LOWER_LEFT = 0;
    static constexpr uint8_t CORNER_UPPER_LEFT = 1;
    static constexpr uint8_t CORNER_UPPER_RIGHT = 2;
    static constexpr uint8_t CORNER_LOWER_RIGHT = 3;

    static constexpr uint8_t EDGE_LEFT = 0;
    static constexpr uint8_t EDGE_TOP = 1;
    static constexpr uint8_t EDGE_RIGHT = 2;
    static constexpr uint8_t EDGE_BOTTOM = 3;

    TriangulatedDisplacement(
      const Structs::DispInfo& dispInfo,
      std::span<const Structs::DispVert> dispVertices,
      std::span<const Structs::Edge> edges,
      std::span<const Structs::Vector> vertices,
      std::span<const int32_t> surfaceEdges,
      const Structs::TexInfo& textureInfo,
      const Structs::TexData& textureData
    );

    Structs::DispInfo dispInfo;
    Structs::TexInfo textureInfo;
    Structs::TexData textureData;

    std::vector<Vertex> vertices;

    size_t numVerticesPerAxis;

    std::array<Structs::DispNeighbour, 4> edgeNeighbours;
    std::array<std::vector<uint16_t>, 4> cornerNeighbours;

    void smoothNeighbouringNormals(std::span<const TriangulatedDisplacement> displacements);

    [[nodiscard]] size_t getTriangleListIndexCount() const;
    void generateTriangleListIndices(const std::function<void(uint32_t i0, uint32_t i1, uint32_t i2)>& iteratee) const;

  private:
    [[nodiscard]] std::vector<Vertex> triangulate(
      std::span<const Structs::DispVert> dispVertices,
      std::span<const Structs::Edge> edges,
      std::span<const Structs::Vector> vertices,
      std::span<const int32_t> surfaceEdges
    ) const;

    void generateInternalNormals();
    [[nodiscard]] Structs::Vector generateInternalNormal(size_t x, size_t y) const;

    [[nodiscard]] size_t getVertexIndex(size_t x, size_t y) const;
    [[nodiscard]] const Vertex& getVertex(size_t x, size_t y) const;
  };
}
