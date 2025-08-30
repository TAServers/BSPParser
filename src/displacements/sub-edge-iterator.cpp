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

  namespace {
    const DispSubNeighbour& getSubNeighbour(
      const TriangulatedDisplacement& displacement, int32_t edgeIndex, int32_t subNeighbourIndex
    ) {
      const auto& subNeighbour = displacement.edgeNeighbours[edgeIndex].subNeighbors[subNeighbourIndex];

      if (!subNeighbour.isValid()) {
        throw std::invalid_argument(
          std::format(
            "Invalid sub-neighbour at edge index '{}' and sub-neighbour index '{}'", edgeIndex, subNeighbourIndex
          )
        );
      }

      return subNeighbour;
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
      const int32_t sideLengthM1 = numVerticesPerAxis - 1;

      switch (corner) {
        case TriangulatedDisplacement::CORNER_UPPER_LEFT:
          return VertexCoordinate{0, sideLengthM1};
        case TriangulatedDisplacement::CORNER_UPPER_RIGHT:
          return VertexCoordinate{sideLengthM1, sideLengthM1};
        case TriangulatedDisplacement::CORNER_LOWER_RIGHT:
          return VertexCoordinate{sideLengthM1, 0};
        default:
          throw std::invalid_argument(std::format("Invalid corner index '{}'", corner));
      }
    }

    std::pair<VertexCoordinate, VertexCoordinate> setupSpan(
      const int32_t numVerticesPerAxis, const int32_t edgeIndex, const uint8_t span
    ) {
      const auto freeDimension =
        EDGE_AXES[edgeIndex] == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;
      const auto midPoint = numVerticesPerAxis / 2;

      auto start = getCornerPointIndex(numVerticesPerAxis, edgeIndex);
      auto end = getCornerPointIndex(numVerticesPerAxis, (edgeIndex + 1) & 3);

      if (edgeIndex == TriangulatedDisplacement::EDGE_RIGHT || edgeIndex == TriangulatedDisplacement::EDGE_BOTTOM) {
        if (span == CORNER_TO_MIDPOINT) {
          start[freeDimension] = midPoint;
        } else if (span == MIDPOINT_TO_CORNER) {
          end[freeDimension] = midPoint;
        }
      } else {
        if (span == CORNER_TO_MIDPOINT) {
          end[freeDimension] = midPoint;
        } else if (span == MIDPOINT_TO_CORNER) {
          start[freeDimension] = midPoint;
        }
      }

      return std::make_pair(start, end);
    }
  }

  VertexCoordinate SubEdgeIterator::transformIntoSubNeighbour(
    const int32_t edgeIndex, const VertexCoordinate& toTransform
  ) const {
    // Not exactly sure what this represents. Extracted into a constant so at least the magic is obvious...
    constexpr auto magic2To16th = 1u << 16u;

    const auto [srcStart, srcEnd] = setupSpan(displacement->numVerticesPerAxis, edgeIndex, neighbourSpan);

    const auto neighbourEdgeIndex = (edgeIndex + 2 + neighbourOrientation) & 3u;
    const auto [destEnd, destStart] = setupSpan(neighbour->numVerticesPerAxis, neighbourEdgeIndex, neighbourSpan);

    const auto freeDimension = EDGE_AXES[edgeIndex];
    const auto fixedPercent = (toTransform[freeDimension] - srcStart[freeDimension]) * magic2To16th /
      (srcEnd[freeDimension] - srcStart[freeDimension]);
    if (fixedPercent < 0 || fixedPercent > magic2To16th) {
      throw std::out_of_range("fixedPercent out of range");
    }

    const auto neighbourEdgeAxis = EDGE_AXES[neighbourEdgeIndex];
    const auto neighbourFreeAxis =
      neighbourEdgeAxis == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;

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

    // Using the shift info actually causes indexing out of range into the neighbour's verts
    // I have tried figuring out how Source doesn't encounter this issue to no avail
    // Removing the shift add below seems to have no effect on the final normals however (in fact they appear smoother)
    // So my only conclusion is that Source literally causes heap corruption in the displacement deserialization inside VRAD
    // If anyone has a better explanation please raise an issue on GitHub
    //const ShiftInfo& shiftInfo = g_ShiftInfos[sub.span][sub.neighbourSpan];
    //if (!shiftInfo.valid) throw std::runtime_error("Shift info invalid");

    VertexCoordinate tempInc;

    const auto sideLength = displacement->numVerticesPerAxis;

    coordinate[edgeAxis] = EDGE_SIDE_LENGTH_MULTIPLIERS[edgeIndex] * (sideLength - 1);
    coordinate[freeAxis] = sideLength / 2 * subNeighbourIndex;
    neighbourCoordinate = transformIntoSubNeighbour(edgeIndex, coordinate);

    const auto power = displacement->dispInfo.power;
    const auto neighbourPower = neighbour->dispInfo.power; // + shiftInfo.powerShiftAdd;

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

    if (neighbourSpan == CORNER_TO_MIDPOINT) {
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
    neighbourSpan(subNeighbour.span) {
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
