#pragma once

#include "../structs/zip.hpp"
#include <span>
#include <string_view>
#include <vector>

namespace BspParser::Zip {
  struct ZipFileEntry {
    Structs::Zip::FileHeader header;
    std::string_view fileName;
    std::span<const std::byte> data;
  };

  std::vector<ZipFileEntry> readZipFileEntries(std::span<const std::byte> zipData);
}
