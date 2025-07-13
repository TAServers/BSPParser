#include "zip.hpp"
#include <limits>
#include <optional>
#include <stdexcept>

namespace BspParser::Zip {
  using namespace Structs::Zip;

  namespace {
    std::optional<EndOfCentralDirectoryRecord> findEndOfCentralDirectoryRecord(
      const std::span<const std::byte> zipData
    ) {
      const auto firstOffset = zipData.size_bytes() - sizeof(EndOfCentralDirectoryRecord);
      const auto lastOffset = firstOffset - std::numeric_limits<uint16_t>::max();

      for (size_t offset = firstOffset; offset >= lastOffset; offset--) {
        const auto& possibleRecord = *reinterpret_cast<const EndOfCentralDirectoryRecord*>(&zipData[offset]);
        const auto bytesAfterRecord = firstOffset - offset;

        if (possibleRecord.signature == EndOfCentralDirectoryRecord::SIGNATURE &&
            possibleRecord.commentLength == bytesAfterRecord) {
          return possibleRecord;
        }
      }

      return std::nullopt;
    }

    std::vector<FileHeader> readFileHeaders(
      const std::span<const std::byte> zipData, const EndOfCentralDirectoryRecord& eocdRecord
    ) {
      std::vector<FileHeader> headers;
      headers.reserve(eocdRecord.numCentralDirectoryEntriesInThisDisk);

      size_t offset = eocdRecord.startOfCentralDirOffset;
      for (uint16_t fileHeaderIndex = 0; fileHeaderIndex < eocdRecord.numCentralDirectoryEntriesInThisDisk;
           fileHeaderIndex++) {
        const auto& header = *reinterpret_cast<const FileHeader*>(&zipData[offset]);
        headers.push_back(header);

        offset += sizeof(FileHeader) + header.fileNameLength + header.extraFieldLength + header.fileCommentLength;
      }

      return std::move(headers);
    }
  }

  std::vector<ZipFileEntry> readZipFileEntries(const std::span<std::byte const> zipData) {
    const auto eocdRecord = findEndOfCentralDirectoryRecord(zipData);
    if (!eocdRecord.has_value()) {
      throw std::runtime_error("Unable to find zip file end of central directory record");
    }

    const auto fileHeaders = readFileHeaders(zipData, eocdRecord.value());

    std::vector<ZipFileEntry> files;
    files.reserve(fileHeaders.size());

    for (const auto& fileHeader : fileHeaders) {
      if (fileHeader.signature != FileHeader::SIGNATURE) {
        throw std::runtime_error("Zip central directory file header signature is invalid");
      }

      const auto& localFileHeader =
        *reinterpret_cast<const LocalFileHeader*>(&zipData[fileHeader.localFileHeaderOffset]);
      if (localFileHeader.signature != LocalFileHeader::SIGNATURE) {
        throw std::runtime_error("Zip local file header signature is invalid");
      }

      const auto endOfLocalHeaderOffset = fileHeader.localFileHeaderOffset + sizeof(LocalFileHeader);

      const auto fileData = zipData.subspan(
        endOfLocalHeaderOffset + localFileHeader.fileNameLength + localFileHeader.extraFieldLength,
        localFileHeader.compressedSize
      );
      const auto fileName = std::string_view(
        reinterpret_cast<const char*>(&zipData[endOfLocalHeaderOffset]), localFileHeader.fileNameLength
      );

      files.push_back(
        ZipFileEntry{
          .header = fileHeader,
          .fileName = fileName,
          .data = fileData,
        }
      );
    }

    return std::move(files);
  }
}
