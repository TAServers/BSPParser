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

  class DisplacementNormals {
  public:
    DisplacementNormals(const Bsp& bsp, const Structs::DispInfo& dispInfo);

    [[nodiscard]] Structs::Vector calculateNormal(size_t x, size_t y) const;

  private:
    std::array<std::vector<Displacement>, 4> edgeNeighbours;
    std::array<std::vector<Displacement>, 4> cornerNeighbours;
  };
}
