#include "face-accessors.hpp"
#include "helpers/vector-maths.hpp"
#include <map>

namespace BspParser::Accessors {
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

    void assertFaceCanBeDirectlyTriangulated(const Structs::Face& face, const std::span<const int32_t> surfaceEdges) {
      if (face.dispInfo >= 0) {
        throw std::runtime_error(
          "Cannot triangulate and draw displacement faces directly. Use the displacement accessors instead or triangulate manually if that's definitely what you want."
        );
      }

      if (surfaceEdges.size() < 3) {
        throw std::runtime_error("Face has less than 3 required edges needed to triangulate");
      }
    }
  }

  void iterateModels(
    const Bsp& bsp,
    const std::function<void(const Structs::Model& model, const std::vector<Bsp::PhysModel>& physicsModels)>& iteratee
  ) {
    std::vector<Bsp::PhysModel> physicsModels;

    for (int32_t modelIndex = 0; modelIndex < bsp.models.size(); modelIndex++) {
      physicsModels.clear();
      for (const auto& physModel : bsp.physicsModels) {
        if (physModel.modelIndex == modelIndex) {
          physicsModels.push_back(physModel);
        }
      }

      iteratee(bsp.models[modelIndex], physicsModels);
    }
  }

  void iterateFaces(
    const Bsp& bsp,
    const Structs::Model& model,
    const std::function<void(
      const Structs::Face& face,
      const Structs::Plane& plane,
      const Structs::TexInfo& textureInfo,
      std::span<const int32_t> surfaceEdges
    )>& iteratee
  ) {
    if (model.numFaces == 0) {
      return;
    }

    if (model.firstFace < 0 || model.firstFace >= bsp.faces.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::Models,
        std::format("Model firstFace index '{}' is out of bounds of the faces lump", model.firstFace)
      );
    }

    if (model.numFaces < 0) {
      throw Errors::InvalidBody(Enums::Lump::Models, "Model's numFaces must be non-negative");
    }

    if (model.firstFace + model.numFaces > bsp.faces.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::Models,
        std::format(
          "Model's firstFace + numFaces ({} + {}) is greater than the size of the faces lump",
          model.firstFace,
          model.numFaces
        )
      );
    }

    for (const auto& face : bsp.faces.subspan(model.firstFace, model.numFaces)) {
      if (face.planeNum >= bsp.planes.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::Faces, std::format("Face plane index '{}' is out of bounds of the plane lump", face.planeNum)
        );
      }

      if (face.texInfo < 0 || face.texInfo >= bsp.textureInfos.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::Faces,
          std::format("Face texture info index '{}' is out of bounds of the texture info lump", face.texInfo)
        );
      }

      if (face.firstEdge < 0 || face.firstEdge >= bsp.surfaceEdges.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::Faces,
          std::format("Face firstEdge index '{}' is out of bounds of the surf edges lump", face.firstEdge)
        );
      }

      if (face.firstEdge + face.numEdges > bsp.surfaceEdges.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::Edges,
          std::format(
            "Face's firstEdge + numEdges ({} + {}) is greater than the size of the edges lump",
            face.firstEdge,
            face.numEdges
          )
        );
      }

      iteratee(
        face,
        bsp.planes[face.planeNum],
        bsp.textureInfos[face.texInfo],
        bsp.surfaceEdges.subspan(face.firstEdge, face.numEdges)
      );
    }
  }

  size_t getFaceVertexCount(const Structs::Face& face, const std::span<const int32_t> surfaceEdges) {
    assertFaceCanBeDirectlyTriangulated(face, surfaceEdges);

    return surfaceEdges.size();
  }

  size_t getFaceTriangleListIndexCount(const Structs::Face& face, const std::span<const int32_t> surfaceEdges) {
    assertFaceCanBeDirectlyTriangulated(face, surfaceEdges);

    return (surfaceEdges.size() - 2) * 3;
  }

  void generateFaceVertices(
    const Bsp& bsp,
    const Structs::Face& face,
    const Structs::Plane& plane,
    const Structs::TexInfo& textureInfo,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  ) {
    assertFaceCanBeDirectlyTriangulated(face, surfaceEdges);

    if (textureInfo.texData < 0 || textureInfo.texData >= bsp.textureDatas.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::TextureInfo,
        std::format(
          "Texture info's texture data index '{}' is out of bounds of the texture data lump", textureInfo.texData
        )
      );
    }

    const auto& textureData = bsp.textureDatas[textureInfo.texData];

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

  void generateFaceTriangleList(
    const Structs::Face& face,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(int32_t i0, int32_t i1, int32_t i2)>& iteratee
  ) {
    assertFaceCanBeDirectlyTriangulated(face, surfaceEdges);

    // First and last edge are ignored as they would create duplicate/degenerate/overlapping triangles
    for (int32_t edgeIndex = 1; edgeIndex < surfaceEdges.size() - 1; edgeIndex++) {
      iteratee(0, edgeIndex, edgeIndex + 1);
    }
  }
}
