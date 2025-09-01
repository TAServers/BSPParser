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
    VertexCoordinate::Axis invert(const VertexCoordinate::Axis axis) {
      return axis == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;
    }

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

    VertexCoordinate getCornerPointIndex(const int32_t numVerticesPerAxis, const int32_t corner) {
      switch (corner) {
        case TriangulatedDisplacement::CORNER_LOWER_LEFT:
          return {0, 0};
        case TriangulatedDisplacement::CORNER_UPPER_LEFT:
          return {0, numVerticesPerAxis - 1};
        case TriangulatedDisplacement::CORNER_UPPER_RIGHT:
          return {numVerticesPerAxis - 1, numVerticesPerAxis - 1};
        case TriangulatedDisplacement::CORNER_LOWER_RIGHT:
          return {numVerticesPerAxis - 1, 0};
        default:
          throw std::out_of_range("Corner index out of range");
      }
    }

    std::pair<VertexCoordinate, VertexCoordinate> setupSpan(
      const int32_t numVerticesPerAxis, const int32_t edgeIndex, const uint8_t span
    ) {
      const auto edgeAxis = EDGE_AXES[edgeIndex];
      const auto freeAxis = invert(edgeAxis);
      const auto midPoint = numVerticesPerAxis / 2;

      auto start = getCornerPointIndex(numVerticesPerAxis, edgeIndex);
      auto end = getCornerPointIndex(numVerticesPerAxis, (edgeIndex + 1) & 3);

      if (edgeIndex == TriangulatedDisplacement::EDGE_RIGHT || edgeIndex == TriangulatedDisplacement::EDGE_BOTTOM) {
        if (span == CORNER_TO_MIDPOINT) {
          start[freeAxis] = midPoint;
        } else if (span == MIDPOINT_TO_CORNER) {
          end[freeAxis] = midPoint;
        }
      } else {
        if (span == CORNER_TO_MIDPOINT) {
          end[freeAxis] = midPoint;
        } else if (span == MIDPOINT_TO_CORNER) {
          start[freeAxis] = midPoint;
        }
      }

      return std::make_pair(start, end);
    }
  }

  VertexCoordinate SubEdgeIterator::transformIntoSubNeighbour(
    const int32_t edgeIndex, const VertexCoordinate& toTransform
  ) {
    // Not exactly sure what this represents. Extracted into a constant so at least the magic is obvious...
    constexpr int32_t magic2To16th = 1u << 16u;

    const auto [srcStart, srcEnd] = setupSpan(displacement->numVerticesPerAxis, edgeIndex, span);

    const auto neighbourEdgeIndex = (edgeIndex + 2 + neighbourOrientation) & 3;
    const auto neighbourEdgeAxis = EDGE_AXES[neighbourEdgeIndex];
    neighbourFreeAxis = invert(neighbourEdgeAxis);

    auto [destEnd, destStart] = setupSpan(neighbour->numVerticesPerAxis, neighbourEdgeIndex, neighbourSpan);

    const auto fixedPercent =
      (toTransform[freeAxis] - srcStart[freeAxis]) * magic2To16th / (srcEnd[freeAxis] - srcStart[freeAxis]);
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
    freeAxis = invert(edgeAxis);

    const ShiftInfo& shiftInfo = SHIFT_INFOS[span][neighbourSpan];
    const auto powerShiftAdd = shiftInfo.valid ? shiftInfo.powerShiftAdd : 0;

    VertexCoordinate tempInc;

    const int32_t sideLength = displacement->numVerticesPerAxis;

    coordinate[edgeAxis] = EDGE_SIDE_LENGTH_MULTIPLIERS[edgeIndex] * (sideLength - 1);
    coordinate[freeAxis] = sideLength / 2 * subNeighbourIndex;
    neighbourCoordinate = transformIntoSubNeighbour(edgeIndex, coordinate);

    const auto power = displacement->dispInfo.power;
    const auto neighbourPower = neighbour->dispInfo.power + powerShiftAdd;

    increment[edgeAxis] = tempInc[edgeAxis] = 0;
    if (neighbourPower > power) {
      increment[freeAxis] = 1;
      tempInc[freeAxis] = 1 << (neighbourPower - power);
    } else {
      increment[freeAxis] = 1 << (power - neighbourPower);
      tempInc[freeAxis] = 1;
    }

    neighbourIncrement = rotateVertexIncrement(neighbourOrientation, tempInc);

    if (span == CORNER_TO_MIDPOINT) {
      end = sideLength >> 1;
    } else {
      end = sideLength - 1;
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
    neighbourCoordinate.x += neighbourIncrement.x;
    neighbourCoordinate.y += neighbourIncrement.y;

    // The source sdk (and original BSPParser) don't check the bounds of the neighbour axis here
    // After the rewrite I was getting out of range index errors from the neighbour coord though,
    // so either something is subtly wrong with the reimplementation, or it always used to index out of range silently
    return coordinate[freeAxis] < end;
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
