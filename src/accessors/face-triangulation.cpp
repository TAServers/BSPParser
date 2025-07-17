#include "face-triangulation.hpp"
#include "../helpers/vector-maths.hpp"

namespace BspParser::Accessors::Internal {
  namespace {
    const Structs::Vector& getVertexPosition(const Bsp& bsp, const int32_t surfaceEdge) {
      const auto edgeIndex = std::abs(surfaceEdge);
      if (edgeIndex >= bsp.edges.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::SurfaceEdges,
          std::format("Surface edge index '{}' is out of bounds of the edges lump", edgeIndex)
        );
      }

      const auto& edge = bsp.edges[edgeIndex];
      const auto firstVertexIndex = surfaceEdge < 0 ? edge.vertices.back() : edge.vertices.front();

      if (firstVertexIndex >= bsp.vertices.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::Edges,
          std::format("Edge vertex index '{}' is out of bounds of the vertices lump", firstVertexIndex)
        );
      }

      return bsp.vertices[firstVertexIndex];
    }

    Structs::Vector4 calculateTangent(const Structs::Vector& normal, const Structs::TexInfo& textureInfo) {
      const auto& sAxis = textureInfo.textureVecs[0];
      const auto& tAxis = textureInfo.textureVecs[1];

      // https://terathon.com/blog/tangent-space.html
      const auto tangent = normalise(sub(xyz(sAxis), mul(normal, dot(normal, xyz(sAxis)))));
      const auto handedness = dot(cross(normal, tangent), xyz(tAxis)) < 0.f ? -1.f : 1.f;

      return Structs::Vector4{
        .x = tangent.x,
        .y = tangent.y,
        .z = tangent.z,
        .w = handedness,
      };
    }

    Structs::Vector2 calculateUvs(
      const Structs::Vector& position, const Structs::TexInfo& textureInfo, const Structs::TexData& textureData
    ) {
      const auto& sAxis = textureInfo.textureVecs[0];
      const auto& tAxis = textureInfo.textureVecs[1];

      return Structs::Vector2{
        .u = (dot(xyz(sAxis), position) + sAxis.w) / static_cast<float>(textureData.width),
        .v = (dot(xyz(tAxis), position) + tAxis.w) / static_cast<float>(textureData.height),
      };
    }
  }

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
    std::span<const int32_t> surfaceEdges, const std::function<void(int32_t i0, int32_t i1, int32_t i2)>& iteratee
  ) {
    // First and last edge are ignored as they would create duplicate/degenerate/overlapping triangles
    for (int32_t edgeIndex = 1; edgeIndex < surfaceEdges.size() - 1; edgeIndex++) {
      iteratee(0, edgeIndex, edgeIndex + 1);
    }
  }
}
