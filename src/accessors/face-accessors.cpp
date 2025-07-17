#include "./face-accessors.hpp"
#include "../helpers/vector-maths.hpp"
#include "./displacements/indices.hpp"
#include "./displacements/vertices.hpp"
#include "./face-triangulation.hpp"

namespace BspParser::Accessors {
  namespace {
    void assertFaceCanBeTriangulated(const std::span<const int32_t> surfaceEdges) {
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

  size_t getVertexCount(const Bsp& bsp, const Structs::Face& face, const std::span<const int32_t> surfaceEdges) {
    assertFaceCanBeTriangulated(surfaceEdges);

    if (face.dispInfo < 0) {
      return surfaceEdges.size();
    }

    const auto& dispInfo = bsp.displacementInfos[face.dispInfo];

    const auto numVerticesPerAxis = (1ul << static_cast<size_t>(dispInfo.power)) + 1;
    return numVerticesPerAxis * numVerticesPerAxis;
  }

  size_t getTriangleListIndexCount(
    const Bsp& bsp, const Structs::Face& face, const std::span<const int32_t> surfaceEdges
  ) {
    assertFaceCanBeTriangulated(surfaceEdges);

    if (face.dispInfo < 0) {
      return (surfaceEdges.size() - 2) * 3;
    }

    const auto& dispInfo = bsp.displacementInfos[face.dispInfo];

    const auto numEdgesPerAxis = 1ul << static_cast<size_t>(dispInfo.power);
    return numEdgesPerAxis * numEdgesPerAxis * 2 * 3; // 2 triangles per quad * 3 vertices per triangle
  }

  void generateVertices(
    const Bsp& bsp,
    const Structs::Face& face,
    const Structs::Plane& plane,
    const Structs::TexInfo& textureInfo,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  ) {
    assertFaceCanBeTriangulated(surfaceEdges);

    if (textureInfo.texData < 0 || textureInfo.texData >= bsp.textureDatas.size()) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::TextureInfo,
        std::format(
          "Texture info's texture data index '{}' is out of bounds of the texture data lump", textureInfo.texData
        )
      );
    }

    const auto& textureData = bsp.textureDatas[textureInfo.texData];

    if (face.dispInfo < 0) {
      Internal::generateFaceVertices(bsp, plane, textureInfo, textureData, surfaceEdges, iteratee);
    } else {
      const auto& dispInfo = bsp.displacementInfos[face.dispInfo];
      Internal::generateDisplacementVertices(bsp, dispInfo, textureInfo, surfaceEdges, iteratee);
    }
  }

  void generateTriangleListIndices(
    const Bsp& bsp,
    const Structs::Face& face,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(uint32_t i0, uint32_t i1, uint32_t i2)>& iteratee
  ) {
    assertFaceCanBeTriangulated(surfaceEdges);

    if (face.dispInfo < 0) {
      Internal::generateFaceTriangleListIndices(surfaceEdges, iteratee);
    } else {
      const auto& dispInfo = bsp.displacementInfos[face.dispInfo];
      Internal::generateDisplacementTriangleListIndices(dispInfo, iteratee);
    }
  }
}
