#pragma once

#include <cstdint>

namespace BspParser::Structs {
  struct Brush {
    int32_t firstSide;
    int32_t numSides;
    int32_t contents;
  };

  struct BrushSide {
    uint16_t planeNum;
    int16_t texInfo;
    int16_t dispInfo;
    int16_t bevel;
  };
}
