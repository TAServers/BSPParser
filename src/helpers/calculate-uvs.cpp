#include "calculate-uvs.hpp"
#include "vector-maths.hpp"

namespace BspParser::Internal {
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
