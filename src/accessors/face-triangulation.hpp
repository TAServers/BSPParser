#pragma once

#include "../bsp.hpp"
#include "./vertex.hpp"
#include <functional>

namespace BspParser::Accessors::Internal {
  void generateFaceVertices(
    const Bsp& bsp,
    const Structs::Plane& plane,
    const Structs::TexInfo& textureInfo,
    const Structs::TexData& textureData,
    std::span<const int32_t> surfaceEdges,
    const std::function<void(const Vertex& vertex)>& iteratee
  );

  void generateFaceTriangleListIndices(
    std::span<const int32_t> surfaceEdges, const std::function<void(int32_t i0, int32_t i1, int32_t i2)>& iteratee
  );
}
