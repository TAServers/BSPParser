#include "indices.hpp"

namespace BspParser::Accessors::Internal {
  void Internal::generateDisplacementTriangleListIndices(
    const Structs::DispInfo& dispInfo, const std::function<void(uint32_t i0, uint32_t i1, uint32_t i2)>& iteratee
  ) {
    const auto size = 1u << static_cast<uint32_t>(dispInfo.power);

    for (uint32_t x = 0; x < size; x++) {
      for (uint32_t y = 0; y < size; y++) {
        const auto bottomLeft = y * (size + 1) + x;
        const auto topLeft = (y + 1) * (size + 1) + x;
        const auto topRight = (y + 1) * (size + 1) + (x + 1);
        const auto bottomRight = y * (size + 1) + (x + 1);

        iteratee(bottomLeft, topLeft, topRight);
        iteratee(bottomLeft, topRight, bottomRight);
      }
    }
  }
}
