#pragma once

#include "../bsp.hpp"
#include "../structs/common.hpp"

namespace BspParser::Accessors::Internal {
  const Structs::Vector& getVertexPosition(const Bsp& bsp, int32_t surfaceEdge);
}
