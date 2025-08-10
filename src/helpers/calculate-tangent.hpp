#pragma once

#include "../structs/common.hpp"
#include "../structs/textures.hpp"

namespace BspParser::Internal {
  Structs::Vector4 calculateTangent(const Structs::Vector& normal, const Structs::TexInfo& textureInfo);
}
