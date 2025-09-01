#pragma once

#include "../bsp.hpp"
#include <functional>

namespace BspParser::Accessors {
  /**
   * Calls the given function for each Structs::TexData in the BSP, along with its path.
   * @param bsp BSP instance.
   * @param iteratee Function to be called.
   */
  void iterateTextures(
    const Bsp& bsp, const std::function<void(const Structs::TexData& texture, const char* path)>& iteratee
  );
}
