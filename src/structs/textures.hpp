#pragma once

#include "common.hpp"
#include "../enums/surface.hpp"
#include <array>

namespace BspParser::Structs {
  struct TexInfo {
    std::array<Vector4, 2> textureVecs;
    std::array<Vector4, 2> lightmapVecs;
    Enums::Surface flags;
    int32_t texData;
  };

  struct TexData {
    Vector reflectivity;
    int32_t nameStringTableId;
    int32_t width, height;
    int32_t viewWidth, viewHeight;
  };
}
