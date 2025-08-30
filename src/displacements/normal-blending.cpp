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

    void blendCorners() {}

    void blendTJunctions() {}

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
      blendCorners();

      for (int edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
        const auto& edgeNeighbour = displacement.edgeNeighbours.at(edgeIndex);

        blendTJunctions();
        blendEdges(displacements, displacement, edgeNeighbour, edgeIndex);
      }
    }
  }
}
