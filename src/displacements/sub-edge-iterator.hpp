#pragma once

// Ripped straight from https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/disp_common.h#L48

#include "./triangulated-displacement.hpp"

namespace BspParser::Internal {
  struct VertexCoordinate {
    enum class Axis : uint8_t { X, Y };

    int32_t x = 0;
    int32_t y = 0;

    int32_t& operator[](const Axis axis) {
      return axis == Axis::X ? x : y;
    }

    const int32_t& operator[](const Axis axis) const {
      return axis == Axis::X ? x : y;
    }
  };

  class SubEdgeIterator {
  public:
    SubEdgeIterator(
      const TriangulatedDisplacement& displacement,
      const Structs::DispSubNeighbour& subNeighbour,
      const TriangulatedDisplacement& neighbour,
      int32_t edgeIndex,
      int32_t subNeighbourIndex,
      bool shouldTouchCorners = false
    );

    bool next();

    [[nodiscard]] const VertexCoordinate& getVertexCoordinate() const;
    [[nodiscard]] int32_t getVertexIndex() const;
    [[nodiscard]] int32_t getNeighbourVertexIndex() const;

    [[nodiscard]] bool isLastVertex() const;

    [[nodiscard]] VertexCoordinate::Axis getFreeAxis() const;

  private:
    const TriangulatedDisplacement* displacement = nullptr;
    const TriangulatedDisplacement* neighbour = nullptr;

    uint8_t neighbourOrientation = 0;

    uint8_t span = 0;
    uint8_t neighbourSpan = 0;

    VertexCoordinate coordinate{};
    VertexCoordinate increment{};

    VertexCoordinate neighbourCoordinate{};
    VertexCoordinate neighbourIncrement{};

    int32_t end = 0;
    VertexCoordinate::Axis freeAxis = VertexCoordinate::Axis::X;

    void setupEdgeIncrements(int32_t edgeIndex, int32_t subNeighbourIndex);
    [[nodiscard]] VertexCoordinate transformIntoSubNeighbour(
      int32_t edgeIndex, const VertexCoordinate& toTransform
    ) const;
  };
}
