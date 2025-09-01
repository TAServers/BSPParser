#pragma once

#include <cstdint>

namespace BspParser::Enums {
  // Size is excessive but matches width in the file
  enum class Surface : int32_t { // NOLINT(*-enum-size)
    None = 0x0,
    Light = 0x1,
    Sky2d = 0x2,
    Sky = 0x4,
    Warp = 0x8,
    Transparent = 0x10,
    NoPortal = 0x20,
    Trigger = 0x40,
    NoDraw = 0x80,
    Hint = 0x100,
    Skip = 0x200,
    NoLight = 0x400,
    BumpLight = 0x800,
    NoShadows = 0x1000,
    NoDecals = 0x2000,
    NoChop = 0x4000,
    Hitbox = 0x8000
  };

  inline Surface operator|(Surface lhs, Surface rhs) {
    return static_cast<Surface>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
  }

  inline Surface& operator|=(Surface& lhs, const Surface rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  inline Surface operator&(Surface lhs, Surface rhs) {
    return static_cast<Surface>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
  }

  inline Surface& operator&=(Surface& lhs, const Surface rhs) {
    lhs = lhs & rhs;
    return lhs;
  }
}
