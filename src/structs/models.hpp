#pragma once

#include "common.hpp"
#include <cstdint>

namespace BspParser::Structs {
  struct Model {
    Vector mins, maxs;
    Vector origin;
    int32_t headNode;
    int32_t firstFace, numFaces;
  };
}
