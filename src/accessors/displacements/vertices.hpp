#pragma once

#include "../../bsp.hpp"
#include "../../structs/displacements.hpp"
#include "../vertex.hpp"
#include <functional>

namespace BspParser::Accessors::Internal {
  void generateDisplacementVertices(
    const Bsp& bsp,
    const Structs::DispInfo& dispInfo,
    const Structs::TexInfo& textureInfo,
    std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  );
}
