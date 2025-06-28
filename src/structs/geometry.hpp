#pragma once

#include "common.hpp"
#include <array>
#include <cstdint>

namespace BspParser::Structs {
  struct Plane {
    Vector normal; // Normal of the plane
    float distance; // Distance from origin
    int32_t type; // Plane axis identifier (unused)
  };

  struct Edge {
    /**
     * Indices of each vertex that makes up this edge
     */
    std::array<uint16_t, 2> vertices;
  };

  struct Face {
    uint16_t planeNum;
    uint8_t side;
    uint8_t onNode;
    int32_t firstEdge;
    int16_t numEdges;
    int16_t texInfo;
    int16_t dispInfo;
    int16_t surfaceVolumeFogId;
    std::array<uint8_t, 4> styles;
    int32_t lightOffset;

    /**
     * Area of the face in hammer units squared
     */
    float area;

    std::array<int32_t, 2> lightmapTextureMinsInLuxels;
    std::array<int32_t, 2> lightmapTextureSizeInLuxels;
    int32_t originalFace;
    uint16_t numPrimitives;
    uint16_t firstPrimitiveId;
    uint32_t smoothingGroups;
  };
}
