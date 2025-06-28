#pragma once

#include "common.hpp"
#include "src/enums/props.hpp"
#include "src/limits.hpp"
#include <array>

namespace BspParser::Structs {
  struct DetailObjectDict {
    std::array<char, Limits::DETAIL_NAME_LENGTH> modelName;
  };

  /**
   * @warning All detail prop sprites must lie in the material detail/detailsprites
   */
  struct DetailSpriteDict {
    Vector2 upperLeft;
    Vector2 lowerRight;
    Vector2 texUpperLeft;
    Vector2 texLowerRight;
  };

  struct DetailObject {
    Vector origin;
    EulerRotation angles;

    /**
     * Either index into DetailObjectDictLump_t or DetailPropSpriteLump_t
     */
    uint16_t detailModel;

    uint16_t leaf;
    ColorRgbExp32 lighting;
    uint32_t lightStyles;
    uint8_t lightStyleCount;

    /**
     * How much do the details sway
     */
    uint8_t swayAmount;

    /**
     * Angle param for shaped sprites
     */
    uint8_t shapeAngle;

    /**
     * Size param for shaped sprites
     */
    uint8_t shapeSize;

    Enums::DetailPropOrientation orientation;
    std::array<uint8_t, 3> padding2;
    Enums::DetailPropType type;
    std::array<uint8_t, 3> padding3;
    float flScale; // For sprites only currently
  };

  struct DetailPropLightstyles {
    ColorRgbExp32 lighting;
    uint8_t style;
  };
}
