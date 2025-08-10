#include "calculate-tangent.hpp"
#include "vector-maths.hpp"

namespace BspParser::Internal {
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
}
