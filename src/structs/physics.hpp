#pragma once

#include <cstdint>

namespace BspParser::Structs {
  struct PhysModelHeader {
    int32_t modelIndex;
    int32_t collisionDataSize;
    int32_t textSectionSize;
    int32_t solidCount;
  };
}
