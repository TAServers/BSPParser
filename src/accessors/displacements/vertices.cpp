#include "vertices.hpp"
#include "displacement.hpp"
#include "../../helpers/vector-maths.hpp"
#include "../calculate-tangent.hpp"

namespace BspParser::Accessors::Internal {
  void generateDisplacementVertices(
    const Bsp& bsp,
    const Structs::DispInfo& dispInfo,
    const Structs::TexInfo& textureInfo,
    const Structs::TexData& textureData,
    const std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  ) {
    const auto displacement = Displacement(bsp, dispInfo, textureInfo, textureData, surfaceEdges);
    const auto normals = DisplacementNormals(bsp, dispInfo);

    for (size_t y = 0; y < displacement.getNumVerticesPerAxis(); y++) {
      for (size_t x = 0; x < displacement.getNumVerticesPerAxis(); x++) {
        const auto normal = normals.calculateNormal(x, y);
        const auto tangent = calculateTangent(normal, textureInfo);

        iteratee(
          Vertex{
            .position = displacement.calculatePosition(x, y),
            .normal = normal,
            .tangent = tangent,
            .uv = displacement.calculateUv(x, y),
            .alpha = displacement.getAlpha(x, y),
          }
        );
      }
    }
  }
}
