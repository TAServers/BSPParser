#include "normal-blending.hpp"
#include "sub-edge-iterator.hpp"
#include "../helpers/vector-maths.hpp"

namespace BspParser::Internal {
  namespace {
    float remap(const float val, const float A, const float B, const float C, const float D) {
      if (A == B) {
        return val >= B ? D : C;
      }

      return C + (D - C) * (val - A) / (B - A);
    }

    std::vector<size_t> getAllNeighbourIndices(const TriangulatedDisplacement& displacement) {
      std::vector<size_t> neighbourIndices;

      for (const auto& corner : displacement.cornerNeighbours) {
        neighbourIndices.insert(neighbourIndices.end(), corner.begin(), corner.end());
      }

      for (const auto& edge : displacement.edgeNeighbours) {
        for (const auto& subNeighbour : edge.subNeighbors) {
          if (subNeighbour.isValid()) {
            neighbourIndices.push_back(subNeighbour.index);
          }
        }
      }

      return std::move(neighbourIndices);
    }

    size_t cornerToVertIdx(const TriangulatedDisplacement& displacement, const int32_t corner) {
      size_t x = 0;
      size_t y = 0;

      if (corner == TriangulatedDisplacement::CORNER_UPPER_LEFT ||
          corner == TriangulatedDisplacement::CORNER_UPPER_RIGHT) {
        y = displacement.numVerticesPerAxis - 1;
      }

      if (corner == TriangulatedDisplacement::CORNER_UPPER_RIGHT ||
          corner == TriangulatedDisplacement::CORNER_LOWER_RIGHT) {
        x = displacement.numVerticesPerAxis - 1;
      }

      return y * displacement.numVerticesPerAxis + x;
    }

    size_t getEdgeMidPoint(const TriangulatedDisplacement& displacement, const int32_t edge) {
      const auto end = displacement.numVerticesPerAxis - 1;
      const auto mid = displacement.numVerticesPerAxis / 2;

      size_t x = 0;
      size_t y = 0;

      switch (edge) {
        case TriangulatedDisplacement::EDGE_LEFT:
          y = mid;
          break;
        case TriangulatedDisplacement::EDGE_TOP:
          x = mid;
          y = end;
          break;
        case TriangulatedDisplacement::EDGE_RIGHT:
          x = end;
          y = mid;
          break;
        case TriangulatedDisplacement::EDGE_BOTTOM:
          x = mid;
          break;
        default:
          break;
      }

      return y * displacement.numVerticesPerAxis + x;
    }

    int32_t findNeighbourCorner(const TriangulatedDisplacement& displacement, const Structs::Vector& test) {
      int32_t closestCorner = 0;
      auto closestDistance = std::numeric_limits<float>::max();

      for (int32_t corner = 0; corner < 4; corner++) {
        const auto cornerVertexIndex = cornerToVertIdx(displacement, corner);

        const auto& cornerVertex = displacement.vertices[cornerVertexIndex];

        const auto delta = sub(cornerVertex.position, test);
        const auto distance = length(delta);

        if (distance < closestDistance) {
          closestCorner = corner;
          closestDistance = distance;
        }
      }

      return closestDistance <= 0.1f ? closestCorner : -1;
    }

    void blendCorners(const std::span<TriangulatedDisplacement> displacements, TriangulatedDisplacement& displacement) {
      const auto neighbourIndices = getAllNeighbourIndices(displacement);
      std::vector neighbourCornerVertexIndices(neighbourIndices.size(), -1);

      for (int32_t corner = 0; corner < 4; corner++) {
        const auto cornerVertexIndex = cornerToVertIdx(displacement, corner);
        auto& cornerVertex = displacement.vertices[cornerVertexIndex];

        auto divisor = 1.f;
        auto averageT = xyz(cornerVertex.tangent);
        auto averageN = cornerVertex.normal;

        for (size_t neighbourIndex = 0; neighbourIndex < neighbourIndices.size(); neighbourIndex++) {
          const auto& neighbour = displacements[neighbourIndices[neighbourIndex]];
          const auto neighbourCorner = findNeighbourCorner(neighbour, cornerVertex.position);

          if (neighbourCorner < 0) {
            neighbourCornerVertexIndices[neighbourIndex] = -1;
            continue;
          }

          const auto neighbourCornerVertexIndex = cornerToVertIdx(neighbour, neighbourCorner);
          const auto& neighbourVertex = neighbour.vertices[neighbourCornerVertexIndex];
          neighbourCornerVertexIndices[neighbourIndex] = neighbourCornerVertexIndex;

          averageT = add(averageT, xyz(neighbourVertex.tangent));
          averageN = add(averageN, neighbourVertex.normal);
          divisor++;
        }

        averageT = div(averageT, divisor);
        averageN = div(averageN, divisor);

        cornerVertex.tangent = Structs::Vector4{averageT.x, averageT.y, averageT.z, cornerVertex.tangent.w};
        cornerVertex.normal = averageN;

        for (size_t neighbourIndex = 0; neighbourIndex < neighbourIndices.size(); neighbourIndex++) {
          const auto vertexIndex = neighbourCornerVertexIndices[neighbourIndex];
          if (vertexIndex < 0) {
            continue;
          }

          auto& vertex = displacements[neighbourIndices[neighbourIndex]].vertices[vertexIndex];
          vertex.tangent = Structs::Vector4{averageT.x, averageT.y, averageT.z, cornerVertex.tangent.w};
          vertex.normal = averageN;
        }
      }
    }

    void blendTJunctions(
      const std::span<TriangulatedDisplacement> displacements,
      TriangulatedDisplacement& displacement,
      const Structs::DispNeighbour& neighbour,
      const int32_t edgeIndex
    ) {
      if (!neighbour.subNeighbors[0].isValid() || !neighbour.subNeighbors[1].isValid()) {
        return;
      }

      const auto midPointVertexIndex = getEdgeMidPoint(displacement, edgeIndex);
      auto& midPoint = displacement.vertices[midPointVertexIndex];

      auto& neighbourA = displacements[neighbour.subNeighbors[0].index];
      auto& neighbourB = displacements[neighbour.subNeighbors[1].index];

      const auto cornerA = findNeighbourCorner(neighbourA, midPoint.position);
      const auto cornerB = findNeighbourCorner(neighbourB, midPoint.position);

      if (cornerA < 0 || cornerB < 0) {
        return;
      }

      auto& cornerAVertex = neighbourA.vertices[cornerToVertIdx(neighbourA, cornerA)];
      auto& cornerBVertex = neighbourB.vertices[cornerToVertIdx(neighbourB, cornerB)];

      const auto averageT = div(add(xyz(midPoint.tangent), xyz(cornerAVertex.tangent), xyz(cornerBVertex.tangent)), 3);
      const auto averageN = div(add(midPoint.normal, cornerAVertex.normal, cornerBVertex.normal), 3);

      midPoint.tangent = cornerAVertex.tangent = cornerBVertex.tangent =
        Structs::Vector4{averageT.x, averageT.y, averageT.z, midPoint.tangent.w};
      midPoint.normal = cornerAVertex.normal = cornerBVertex.normal = averageN;
    }

    void blendEdges(
      const std::span<TriangulatedDisplacement> displacements,
      TriangulatedDisplacement& displacement,
      const Structs::DispNeighbour& neighbour,
      const int32_t edgeIndex
    ) {
      for (int32_t subNeighbourIndex = 0; subNeighbourIndex < 2; subNeighbourIndex++) {
        const auto subNeighbour = neighbour.subNeighbors.at(subNeighbourIndex);
        if (!subNeighbour.isValid()) {
          continue;
        }

        auto& neighbourDisplacement = displacements[subNeighbour.index];

        SubEdgeIterator iterator(displacement, subNeighbour, neighbourDisplacement, edgeIndex, subNeighbourIndex, true);
        const auto freeAxis = iterator.getFreeAxis();
        const auto edgeAxis =
          freeAxis == VertexCoordinate::Axis::X ? VertexCoordinate::Axis::Y : VertexCoordinate::Axis::X;

        iterator.next();
        auto previousPos = iterator.getVertexCoordinate();

        while (iterator.next()) {
          if (!iterator.isLastVertex()) {
            auto& vertex = displacement.vertices[iterator.getVertexIndex()];
            auto& neighbourVertex = neighbourDisplacement.vertices[iterator.getNeighbourVertexIndex()];

            // TODO #174: Do we need to handle different handedness?
            const auto averageT = div(add(xyz(vertex.tangent), xyz(neighbourVertex.tangent)), 2);
            const auto averageN = div(add(vertex.normal, neighbourVertex.normal), 2);

            vertex.tangent = Structs::Vector4{averageT.x, averageT.y, averageT.z, vertex.tangent.w};
            vertex.normal = averageN;
            neighbourVertex.tangent = Structs::Vector4{averageT.x, averageT.y, averageT.z, neighbourVertex.tangent.w};
            neighbourVertex.normal = averageN;
          }

          const auto previousPosFreeAxis = previousPos[freeAxis];
          const auto currentPosFreeAxis = iterator.getVertexCoordinate()[freeAxis];

          for (int32_t tweenPos = previousPosFreeAxis + 1; tweenPos < currentPosFreeAxis; tweenPos++) {
            const auto percent = remap(
              static_cast<float>(tweenPos),
              static_cast<float>(previousPosFreeAxis),
              static_cast<float>(currentPosFreeAxis),
              0,
              1
            );
            const auto prevVertexIndex = previousPos.y * displacement.numVerticesPerAxis + previousPos.x;
            auto& prevVertex = displacement.vertices[prevVertexIndex];
            const auto& currVertex = displacement.vertices[iterator.getVertexIndex()];

            const auto tangent = normalise(lerp(xyz(prevVertex.tangent), xyz(currVertex.tangent), percent));
            const auto normal = normalise(lerp(prevVertex.normal, currVertex.normal, percent));

            VertexCoordinate tweenVertex;
            tweenVertex[edgeAxis] = iterator.getVertexCoordinate()[edgeAxis];
            tweenVertex[freeAxis] = tweenPos;

            auto& destVertex = displacement.vertices[tweenVertex.y * displacement.numVerticesPerAxis + tweenVertex.x];
            destVertex.tangent = Structs::Vector4{tangent.x, tangent.y, tangent.z, currVertex.tangent.w};
            destVertex.normal = normal;
          }

          previousPos = iterator.getVertexCoordinate();
        }
      }
    }
  }

  void blendNeighbouringDisplacementNormals(const std::span<TriangulatedDisplacement> displacements) {
    for (auto& displacement : displacements) {
      blendCorners(displacements, displacement);

      for (int edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
        const auto& edgeNeighbour = displacement.edgeNeighbours.at(edgeIndex);

        blendTJunctions(displacements, displacement, edgeNeighbour, edgeIndex);
        blendEdges(displacements, displacement, edgeNeighbour, edgeIndex);
      }
    }
  }
}
