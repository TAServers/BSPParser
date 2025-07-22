#pragma once

#include "../../bsp.hpp"
#include <span>

namespace BspParser::Accessors::Internal {
  class Displacement {
  public:
    Displacement(
      const Bsp& bsp,
      const Structs::DispInfo& dispInfo,
      const Structs::TexInfo& textureInfo,
      const Structs::TexData& textureData,
      std::span<const int32_t> surfaceEdges
    );

    [[nodiscard]] size_t getNumVerticesPerAxis() const;

    [[nodiscard]] Structs::Vector calculatePosition(size_t x, size_t y) const;

    [[nodiscard]] Structs::Vector2 calculateUv(size_t x, size_t y) const;

    [[nodiscard]] float getAlpha(size_t x, size_t y) const;

  private:
    size_t numVerticesPerAxis;
    float edgeLengthFraction;

    std::span<const Structs::DispVert> vertices;

    std::array<Structs::Vector, 4> cornerPositions;
    std::array<Structs::Vector2, 4> cornerUvs;

    std::array<Structs::Vector, 2> positionIncrements;
    std::array<Structs::Vector2, 2> uvIncrements;
  };

  class DisplacementWithNeighbours final : public Displacement {
  public:
    DisplacementWithNeighbours(
      const Bsp& bsp,
      const Structs::DispInfo& dispInfo,
      const Structs::TexInfo& textureInfo,
      const Structs::TexData& textureData,
      std::span<const int32_t> surfaceEdges
    );

    [[nodiscard]] Structs::Vector calculateNormal(size_t x, size_t y) const;

  private:
    static constexpr uint8_t CORNER_LOWER_LEFT = 0;
    static constexpr uint8_t CORNER_UPPER_LEFT = 1;
    static constexpr uint8_t CORNER_UPPER_RIGHT = 2;
    static constexpr uint8_t CORNER_LOWER_RIGHT = 3;

    static constexpr uint8_t EDGE_LEFT = 0;
    static constexpr uint8_t EDGE_TOP = 1;
    static constexpr uint8_t EDGE_RIGHT = 2;
    static constexpr uint8_t EDGE_BOTTOM = 3;

    std::array<std::vector<Displacement>, 4> edgeNeighbours;
    std::array<std::vector<Displacement>, 4> cornerNeighbours;

    [[nodiscard]] std::optional<Structs::Vector> findLeftEdgeVertex(size_t x, size_t y) const;
    [[nodiscard]] std::optional<Structs::Vector> findRightEdgeVertex(size_t x, size_t y) const;
    [[nodiscard]] std::optional<Structs::Vector> findTopEdgeVertex(size_t x, size_t y) const;
    [[nodiscard]] std::optional<Structs::Vector> findBottomEdgeVertex(size_t x, size_t y) const;
  };
}
