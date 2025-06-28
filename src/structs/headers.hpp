#pragma once

#include "src/enums/lump.hpp"
#include <array>
#include <cstdint>

namespace BspParser::Structs {
  constexpr int32_t IDBSPHEADER = 'V' + ('B' << 8u) + ('S' << 16u) + ('P' << 24u);
  constexpr size_t HEADER_LUMPS = 64;

  struct Lump {
    /**
     * Byte offset into file
     */
    int32_t offset;

    /**
     * Length of lump data
     */
    int32_t length;

    /**
     * Lump format version
     */
    int32_t version;

    /**
     * Uncompressed size, or 0 if not compressed
     */
    int32_t fourCC;
  };

  struct Header {
    /**
     * BSP file identifier (should always equal "VBSP")
     */
    int32_t identifier;

    /**
     * Version of the BSP file (19-21)
     */
    int32_t version;

    /**
     * Lump directory
     */
    std::array<Lump, HEADER_LUMPS> lumps;

    /**
     * Map version number
     */
    int32_t mapRevision;
  };

  struct GameLump {
    Enums::GameLumpID id;
    uint16_t flags;
    uint16_t version;
    int32_t offset;
    int32_t length;
  };
}
