#pragma once

#include "../structs/common.hpp"
#include "../structs/geometry.hpp"
#include <span>

namespace BspParser::Internal {
  const Structs::Vector& getVertexPosition(
    std::span<const Structs::Edge> edges, std::span<const Structs::Vector> vertices, int32_t surfaceEdge
  );
}
