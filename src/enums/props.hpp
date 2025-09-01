#pragma once

#include <cstdint>

namespace BspParser::Enums {
  enum class DetailPropOrientation : uint8_t {
    Normal = 0,
    ScreenAligned,
    ScreenAlignedVertical,
  };

  enum class DetailPropType : uint8_t {
    Model = 0,
    Sprite,
    ShapeCross,
    ShapeTriangle,
  };

  enum class StaticPropFlag : uint8_t {
    // Flags field
    // These are automatically computed
    FlagFades = 0x1,
    UseLightingOrigin = 0x2,
    NoDraw = 0x4, // computed at run time based on dx level

    // These are set in WC
    IgnoreNormals = 0x8,
    NoShadow = 0x10,
    ScreenSpaceFade = 0x20,

    NoPerVertexLighting = 0x40, // in vrad, compute lighting at lighting origin, not for each vertex

    NoSelfShadowing = 0x80, // disable self shadowing in vrad

    WcMask = 0xd8 // all flags settable in hammer (?)
  };
}
