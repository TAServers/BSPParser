#include "./face-triangulation.hpp"
#include "../helpers/vector-maths.hpp"
#include "./calculate-tangent.hpp"
#include "./calculate-uvs.hpp"
#include "./get-vertex-position.hpp"

namespace BspParser::Accessors::Internal {
  void generateFaceVertices(
    const Bsp& bsp,
    const Structs::Plane& plane,
    const Structs::TexInfo& textureInfo,
    const Structs::TexData& textureData,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  ) {
    // Dev wiki says face.side is non-zero when the plane faces into the face, but inverting the normal based on that produces incorrect results
    const auto normal = plane.normal;

    for (const auto& surfEdge : surfaceEdges) {
      const auto& position = getVertexPosition(bsp, surfEdge);

      iteratee(
        Vertex{
          .position = position,
          .normal = normal,
          .tangent = calculateTangent(normal, textureInfo),
          .uv = calculateUvs(position, textureInfo, textureData),
        }
      );
    }
  }

  void generateFaceTriangleListIndices(
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(uint32_t i0, uint32_t i1, uint32_t i2)>& iteratee
  ) {
    // First and last edge are ignored as they would create duplicate/degenerate/overlapping triangles
    for (uint32_t edgeIndex = 1; edgeIndex < surfaceEdges.size() - 1; edgeIndex++) {
      iteratee(0, edgeIndex, edgeIndex + 1);
    }
  }
}
