#pragma once

#include "common.hpp"
#include "limits.hpp"
#include "enums/props.hpp"
#include <array>

namespace BspParser::Structs {
  struct StaticPropDict {
    std::array<char, Limits::STATIC_PROP_NAME_LENGTH> modelName;
  };

  struct StaticPropV4 {
    Vector origin;
    EulerRotation angles;
    uint16_t propType;
    uint16_t firstLeaf;
    uint16_t leafCount;
    uint8_t solid;
    Enums::StaticPropFlag flags;
    int32_t skin;
    float fadeMinDist;
    float fadeMaxDist;
    Vector lightingOrigin;
  };

  struct StaticPropV5 {
    Vector origin;
    EulerRotation angles;
    uint16_t propType;
    uint16_t firstLeaf;
    uint16_t leafCount;
    uint8_t solid;
    Enums::StaticPropFlag flags;
    int32_t skin;
    float fadeMinDist;
    float fadeMaxDist;
    Vector lightingOrigin;
    float flForcedFadeScale;
  };

  struct StaticPropV6 {
    Vector origin;
    EulerRotation angles;
    uint16_t propType;
    uint16_t firstLeaf;
    uint16_t leafCount;
    uint8_t solid;
    Enums::StaticPropFlag flags;
    int32_t skin;
    float fadeMinDist;
    float fadeMaxDist;
    Vector lightingOrigin;
    float flForcedFadeScale;
    uint16_t minDXLevel;
    uint16_t maxDXLevel;
  };

  struct StaticPropV7Multiplayer2013 {
    Vector origin;
    EulerRotation angles;
    uint16_t propType;
    uint16_t firstLeaf;
    uint16_t leafCount;
    uint8_t solid;
    int32_t skin;
    float fadeMinDist;
    float fadeMaxDist;
    Vector lightingOrigin;
    float flForcedFadeScale;
    uint16_t minDXLevel;
    uint16_t maxDXLevel;
    uint32_t flags;
    uint8_t LightmapResX;
    uint8_t lightmapResY;
  };

  struct StaticPropLeaf {
    uint16_t leaf;
  };

  struct StaticPropLightstyles {
    ColourRgbExp32 lighting;
  };
}
