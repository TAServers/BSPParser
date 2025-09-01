#pragma once
#include <cstdint>

namespace BspParser::Enums {
  /**
   * https://www.iana.org/assignments/media-types/application/zip
   */
  enum class ZipCompressionMethod : uint16_t {
    Uncompressed,
    Shrunk,
    CompressionFactor1,
    CompressionFactor2,
    CompressionFactor3,
    CompressionFactor4,
    Imploded,
    ReservedForTokenisingCompressionAlgorithm, // ???
    Deflated,
  };
}
