#pragma once

#include "../../structs/displacements.hpp"
#include <functional>

namespace BspParser::Accessors::Internal {
  void generateDisplacementTriangleListIndices(
    const Structs::DispInfo& dispInfo, const std::function<void(uint32_t i0, uint32_t i1, uint32_t i2)>& iteratee
  );
}
