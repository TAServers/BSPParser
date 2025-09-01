#pragma once

#include <cstdint>
#include <span>

namespace BspParser {
  /**
   * Raw physmesh solid and text data for a given model.
   */
  struct PhysModel {
    /**
     * Index into the model lump this physics model is for
     */
    int32_t modelIndex = 0;

    /**
     * Total number of solids in the collision surface sections
     */
    int32_t solidCount = 0;

    /**
     * Raw .PHY solid data. Use `PhyParser::parseSurfaces` from the accompanying PHYParser library to parse this.
     */
    std::span<const std::byte> collisionData;

    /**
     * Raw .PHY text section data. Use `PhyParser::parseTextSection` from the accompanying PHYParser library to parse this.
     */
    std::span<const std::byte> textSectionData;
  };
}
