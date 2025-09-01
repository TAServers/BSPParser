#pragma once

#include "../errors.hpp"
#include <cstddef>

namespace BspParser::Internal {
  inline void checkBounds(const size_t offset, const size_t count, const size_t rangeSize, const char* errorMessage) {
    if (offset >= rangeSize || offset + count > rangeSize) {
      throw Errors::OutOfBoundsAccess(Enums::Lump::None, errorMessage);
    }
  }
}
