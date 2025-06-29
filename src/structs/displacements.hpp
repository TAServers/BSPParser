#pragma once

#include "common.hpp"
#include "limits.hpp"
#include <array>
#include <cstdint>

namespace BspParser::Structs {
  struct DispSubNeighbour {
    uint16_t index = 0xFFFF;
    uint8_t orientation = 0;
    uint8_t span = 0;
    uint8_t neighbourSpan = 0;

    [[nodiscard]] bool isValid() const {
      return index != 0xFFFF;
    }
  };

  struct DispNeighbour {
    std::array<DispSubNeighbour, 2> subNeighbors;
  };

  struct DispCornerNeighbours {
    std::array<uint16_t, Limits::MAX_DISP_CORNER_NEIGHBORS> neighbours;
    uint8_t numNeighbours;
  };

  struct DispInfo {
    Vector startPosition;

    int32_t dispVertStart;
    int32_t dispTriStart;

    int32_t power;
    int32_t minTess;

    float smoothingAngle;
    int32_t contents;
    uint16_t mapFace;
    int32_t lightmapAlphaStart;
    int32_t lightmapSamplePositionStart;

    std::array<DispNeighbour, 4> edgeNeighbours;
    std::array<DispCornerNeighbours, 4> cornerNeighbours;

    std::array<uint32_t, 10> allowedVertices;
  };

  struct DispVert {
    Vector vec;
    float dist;
    float alpha;
  };
}
