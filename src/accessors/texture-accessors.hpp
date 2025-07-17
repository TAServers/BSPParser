#pragma once

#include "../bsp.hpp"
#include <functional>

namespace BspParser::Accessors {
  void iterateTextures(
    const Bsp& bsp, const std::function<void(const Structs::TexData& texture, const char* path)>& iteratee
  );
}
