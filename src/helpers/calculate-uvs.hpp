#pragma once

#include "../structs/common.hpp"
#include "../structs/textures.hpp"

namespace BspParser::Internal {
  Structs::Vector2 calculateUvs(
    const Structs::Vector& position, const Structs::TexInfo& textureInfo, const Structs::TexData& textureData
  );
}
