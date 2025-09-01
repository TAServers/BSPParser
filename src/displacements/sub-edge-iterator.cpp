#include "sub-edge-iterator.hpp"
#include <format>
#include <stdexcept>

namespace BspParser::Internal {
  using namespace Structs;

  constexpr uint8_t CORNER_TO_CORNER = 0;
  constexpr uint8_t CORNER_TO_MIDPOINT = 1;
  constexpr uint8_t MIDPOINT_TO_CORNER = 2;

  constexpr uint8_t ORIENTATION_CCW_0 = 0;
  constexpr uint8_t ORIENTATION_CCW_90 = 1;
  constexpr uint8_t ORIENTATION_CCW_180 = 2;
  constexpr uint8_t ORIENTATION_CCW_270 = 3;

  constexpr auto EDGE_AXES = std::array{
    VertexCoordinate::Axis::X, // EDGE_LEFT
    VertexCoordinate::Axis::Y, // EDGE_TOP
    VertexCoordinate::Axis::X, // EDGE_RIGHT
    VertexCoordinate::Axis::Y, // EDGE_BOTTOM
  };

  constexpr auto EDGE_SIDE_LENGTH_MULTIPLIERS = std::array{0, 1, 1, 0};

  struct ShiftInfo {
    int32_t midPointScale;
    int32_t powerShiftAdd;
    bool valid;
  };

  constexpr std::array SHIFT_INFOS = {
    std::array{
      ShiftInfo{0, 0, true}, // CORNER_TO_CORNER -> CORNER_TO_CORNER
      ShiftInfo{0, -1, true}, // CORNER_TO_CORNER -> CORNER_TO_MIDPOINT
      ShiftInfo{2, -1, true} // CORNER_TO_CORNER -> MIDPOINT_TO_CORNER
    },
    std::array{
      ShiftInfo{0, 1, true}, // CORNER_TO_MIDPOINT -> CORNER_TO_CORNER
      ShiftInfo{0, 0, false}, // CORNER_TO_MIDPOINT -> CORNER_TO_MIDPOINT (invalid)
      ShiftInfo{0, 0, false} // CORNER_TO_MIDPOINT -> MIDPOINT_TO_CORNER (invalid)
    },
    std::array{
      ShiftInfo{-1, 1, true}, // MIDPOINT_TO_CORNER -> CORNER_TO_CORNER
      ShiftInfo{0, 0, false}, // MIDPOINT_TO_CORNER -> CORNER_TO_MIDPOINT (invalid)
      ShiftInfo{0, 0, false} // MIDPOINT_TO_CORNER -> MIDPOINT_TO_CORNER (invalid)
    }
  };

  namespace {
    VertexCoordinate rotateVertexIncrement(const uint8_t orientation, const VertexCoordinate& toRotate) {
      switch (orientation) {
        case ORIENTATION_CCW_0:
          return toRotate;
        case ORIENTATION_CCW_90:
          return VertexCoordinate{toRotate.y, -toRotate.x};
        case ORIENTATION_CCW_180:
          return VertexCoordinate{-toRotate.x, -toRotate.y};
        default:
          return VertexCoordinate{-toRotate.y, toRotate.x};
      }
    }

    std::pair<VertexCoordinate, VertexCoordinate> setupSpan(
      const int32_t numVerticesPerAxis, const int32_t edgeIndex, const uint8_t span
    ) {
      const auto edgeAxis = EDGE_AXES[edgeIndex];
      const auto freeAxis =
        edgeAxis == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;
      const auto midPoint = numVerticesPerAxis / 2;

      VertexCoordinate start{};
      start[edgeAxis] = EDGE_SIDE_LENGTH_MULTIPLIERS[edgeIndex] * (numVerticesPerAxis - 1);
      start[freeAxis] = 0;

      VertexCoordinate end{};
      end[edgeAxis] = EDGE_SIDE_LENGTH_MULTIPLIERS[edgeIndex] * (numVerticesPerAxis - 1);
      end[freeAxis] = numVerticesPerAxis - 1;

      if (span == CORNER_TO_MIDPOINT) {
        end[freeAxis] = midPoint;
      } else if (span == MIDPOINT_TO_CORNER) {
        start[freeAxis] = midPoint;
      }

      return std::make_pair(start, end);
    }
  }

  VertexCoordinate SubEdgeIterator::transformIntoSubNeighbour(
    const int32_t edgeIndex, const VertexCoordinate& toTransform
  ) {
    // Not exactly sure what this represents. Extracted into a constant so at least the magic is obvious...
    constexpr auto magic2To16th = 1u << 16u;

    const auto [srcStart, srcEnd] = setupSpan(displacement->numVerticesPerAxis, edgeIndex, span);

    const auto neighbourEdgeIndex = (edgeIndex + 2 + neighbourOrientation) % 4;
    const auto neighbourEdgeAxis = EDGE_AXES[neighbourEdgeIndex];
    neighbourFreeAxis =
      neighbourEdgeAxis == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;

    auto [destStart, destEnd] = setupSpan(neighbour->numVerticesPerAxis, neighbourEdgeIndex, neighbourSpan);

    if (neighbourIncrement[neighbourFreeAxis] < 0 && destStart[neighbourFreeAxis] < destEnd[neighbourFreeAxis]) {
      std::swap(destStart, destEnd);
    } else if (destStart[neighbourFreeAxis] > destEnd[neighbourFreeAxis]) {
      std::swap(destStart, destEnd);
    }

    const auto freeDimension =
      EDGE_AXES[edgeIndex] == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;
    const auto fixedPercent = (toTransform[freeDimension] - srcStart[freeDimension]) * magic2To16th /
      (srcEnd[freeDimension] - srcStart[freeDimension]);
    if (fixedPercent < 0 || fixedPercent > magic2To16th) {
      throw std::out_of_range("fixedPercent out of range");
    }

    VertexCoordinate out{};
    out[neighbourEdgeAxis] = destStart[neighbourEdgeAxis];
    out[neighbourFreeAxis] = destStart[neighbourFreeAxis] +
      (destEnd[neighbourFreeAxis] - destStart[neighbourFreeAxis]) * fixedPercent / magic2To16th;

    if (out.x < 0 || out.x >= neighbour->numVerticesPerAxis) {
      throw std::out_of_range("x out of bounds in neighbour");
    }
    if (out.y < 0 || out.y >= neighbour->numVerticesPerAxis) {
      throw std::out_of_range("y out of bounds in neighbour");
    }

    return out;
  }

  void SubEdgeIterator::setupEdgeIncrements(const int32_t edgeIndex, const int32_t subNeighbourIndex) {
    const auto edgeAxis = EDGE_AXES[edgeIndex];
    freeAxis = edgeAxis == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;

    const ShiftInfo& shiftInfo = SHIFT_INFOS[span][neighbourSpan];
    const auto powerShiftAdd = shiftInfo.valid ? shiftInfo.powerShiftAdd : 0;

    const auto power = displacement->dispInfo.power;
    const auto neighbourPower = neighbour->dispInfo.power + powerShiftAdd;

    VertexCoordinate tempInc;
    increment[edgeAxis] = 0;
    tempInc[edgeAxis] = 0;
    if (neighbourPower > power) {
      increment[freeAxis] = 1;
      tempInc[freeAxis] = 1u << (neighbourPower - power);
    } else {
      increment[freeAxis] = 1u << (power - neighbourPower);
      tempInc[freeAxis] = 1;
    }

    neighbourIncrement = rotateVertexIncrement(neighbourOrientation, tempInc);

    const auto sideLength = displacement->numVerticesPerAxis;

    coordinate[edgeAxis] = EDGE_SIDE_LENGTH_MULTIPLIERS[edgeIndex] * (sideLength - 1);
    coordinate[freeAxis] = sideLength / 2 * subNeighbourIndex;
    neighbourCoordinate = transformIntoSubNeighbour(edgeIndex, coordinate);

    if (span == CORNER_TO_MIDPOINT) {
      end = sideLength >> 1u;
    } else {
      end = sideLength - 1u;
    }
  }

  SubEdgeIterator::SubEdgeIterator(
    const TriangulatedDisplacement& displacement,
    const DispSubNeighbour& subNeighbour,
    const TriangulatedDisplacement& neighbour,
    const int32_t edgeIndex,
    const int32_t subNeighbourIndex,
    const bool shouldTouchCorners
  ) :
    displacement(&displacement), //
    neighbour(&neighbour), //
    neighbourOrientation(subNeighbour.orientation), //
    span(subNeighbour.span), //
    neighbourSpan(subNeighbour.neighbourSpan) {
    setupEdgeIncrements(edgeIndex, subNeighbourIndex);

    if (shouldTouchCorners) {
      coordinate.x -= increment.x;
      coordinate.y -= increment.y;
      neighbourCoordinate.x -= neighbourIncrement.x;
      neighbourCoordinate.y -= neighbourIncrement.y;

      end += increment[freeAxis];
    }
  }

  bool SubEdgeIterator::next() {
    coordinate.x += increment.x;
    coordinate.y += increment.y;

    neighbourCoordinate.x =
      std::min(neighbourCoordinate.x + neighbourIncrement.x, static_cast<int32_t>(neighbour->numVerticesPerAxis) - 1);
    neighbourCoordinate.y =
      std::min(neighbourCoordinate.y + neighbourIncrement.y, static_cast<int32_t>(neighbour->numVerticesPerAxis) - 1);

    // The source sdk (and original BSPParser) don't check the bounds of the neighbour axis here
    // After the rewrite I was getting out of range index errors from the neighbour coord though,
    // so either something is subtly wrong with the reimplementation, or it always used to index out of range silently
    return coordinate[freeAxis] < end //
      && coordinate[freeAxis] < displacement->numVerticesPerAxis //
      && neighbourCoordinate[neighbourFreeAxis] < neighbour->numVerticesPerAxis;
  }

  const VertexCoordinate& SubEdgeIterator::getVertexCoordinate() const {
    return coordinate;
  }

  int32_t SubEdgeIterator::getVertexIndex() const {
    return coordinate.y * displacement->numVerticesPerAxis + coordinate.x;
  }

  int32_t SubEdgeIterator::getNeighbourVertexIndex() const {
    return neighbourCoordinate.y * neighbour->numVerticesPerAxis + neighbourCoordinate.x;
  }

  bool SubEdgeIterator::isLastVertex() const {
    return coordinate[freeAxis] + increment[freeAxis] >= end;
  }

  VertexCoordinate::Axis SubEdgeIterator::getFreeAxis() const {
    return freeAxis;
  }
}
