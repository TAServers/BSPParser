#pragma once

#include "../enums/zip.hpp"
#include <cstdint>

/**
 * https://medium.com/@felixstridsberg/the-zip-file-format-6c8a160d1c34
 */
namespace BspParser::Structs::Zip {
#pragma pack(push, 1)

  struct EndOfCentralDirectoryRecord {
    static constexpr auto SIGNATURE = 0x06054b50;

    uint32_t signature;
    uint16_t thisDiskNumber;
    uint16_t diskOfCentralDirectoryStart;
    uint16_t numCentralDirectoryEntriesInThisDisk;
    uint16_t numCentralDirectoryEntriesTotal;
    uint32_t centralDirectorySizeBytes;
    uint32_t startOfCentralDirOffset;
    uint16_t commentLength;
  };

  struct FileHeader {
    static constexpr auto SIGNATURE = 0x02014b50;

    uint32_t signature;
    uint16_t versionMadeBy;
    uint16_t versionNeededToExtract;
    uint16_t flags;
    Enums::ZipCompressionMethod compressionMethod;
    uint16_t lastModifiedTime;
    uint16_t lastModifiedDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength;
    uint16_t fileCommentLength;
    uint16_t diskNumberStart;
    uint16_t internalFileAttributes;
    uint32_t externalFileAttributes;
    uint32_t localFileHeaderOffset; // From start of disk
  };

  struct LocalFileHeader {
    static constexpr auto SIGNATURE = 0x04034b50;

    uint32_t signature;
    uint16_t versionNeededToExtract;
    uint16_t flags;
    Enums::ZipCompressionMethod compressionMethod;
    uint16_t lastModifiedTime;
    uint16_t lastModifiedDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength;
  };

#pragma pack(pop)
}
