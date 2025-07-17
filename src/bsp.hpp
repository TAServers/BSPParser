#pragma once

#include "errors.hpp"
#include "enums/lump.hpp"
#include "helpers/offset-data-view.hpp"
#include "helpers/zip.hpp"
#include "structs/common.hpp"
#include "structs/detail-props.hpp"
#include "structs/displacements.hpp"
#include "structs/geometry.hpp"
#include "structs/headers.hpp"
#include "structs/models.hpp"
#include "structs/static-props.hpp"
#include "structs/textures.hpp"
#include <format>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace BspParser {
  struct Bsp {
    struct PhysModel {
      /**
       * Index into the model lump this physics model is for
       */
      int32_t modelIndex;

      /**
       * Total number of solids in the collision surface sections
       */
      int32_t solidCount;

      std::span<const std::byte> collisionData;

      std::span<const std::byte> textSectionData;
    };

    explicit Bsp(std::span<const std::byte> data);

    std::span<const std::byte> data;

    const Structs::Header* header = nullptr;

    std::span<const Structs::GameLump> gameLumps;

    std::span<const Structs::Vector> vertices;
    std::span<const Structs::Plane> planes;
    std::span<const Structs::Edge> edges;
    std::span<const int32_t> surfaceEdges;
    std::span<const Structs::Face> faces;

    std::span<const Structs::TexInfo> textureInfos;
    std::span<const Structs::TexData> textureDatas;
    std::span<const int32_t> textureStringTable;
    std::span<const char> textureStringData;

    std::span<const Structs::Model> models;

    std::span<const Structs::DispInfo> displacementInfos;
    std::span<const Structs::DispVert> displacementVertices;

    std::vector<PhysModel> physicsModels;

    std::vector<Zip::ZipFileEntry> compressedPakfile;

    // std::span<const Structs::DetailObjectDict> detailObjectDictionary;
    // std::span<const Structs::DetailObject> detailObjects;

    std::optional<std::span<const Structs::StaticPropDict>> staticPropDictionary = std::nullopt;
    std::optional<std::span<const Structs::StaticPropLeaf>> staticPropLeaves = std::nullopt;

    std::optional<std::variant<
      std::span<const Structs::StaticPropV4>,
      std::span<const Structs::StaticPropV5>,
      std::span<const Structs::StaticPropV6>,
      std::span<const Structs::StaticPropV7Multiplayer2013>>>
      staticProps = std::nullopt;

  private:
    template <typename LumpType>
    std::span<const LumpType> parseLump(Enums::Lump lump, size_t maxItems = std::numeric_limits<size_t>::max()) {
      const auto& lumpHeader = header->lumps.at(static_cast<size_t>(lump));

      assertLumpHeaderValid(lump, lumpHeader);

      if (lumpHeader.length % sizeof(LumpType) != 0) {
        throw Errors::InvalidBody(
          lump,
          std::format(
            "Lump header has length ({}) which is not a multiple of the size of its item type ({})",
            lumpHeader.length,
            sizeof(LumpType)
          )
        );
      }

      const auto numItems = lumpHeader.length / sizeof(LumpType);
      if (numItems > maxItems) {
        throw Errors::InvalidBody(
          lump, std::format("Number of lump items ({}) exceeds source engine maximum ({})", numItems, maxItems)
        );
      }

      return std::span<const LumpType>(reinterpret_cast<const LumpType*>(&data[lumpHeader.offset]), numItems);
    }

    [[nodiscard]] std::span<const Structs::GameLump> parseGameLumpHeaders() const;

    [[nodiscard]] std::vector<PhysModel> parsePhysCollideLump() const;

    template <class StaticProp>
    [[nodiscard]] std::span<const StaticProp> parseStaticPropLump(const Structs::GameLump& lumpHeader) {
      if (lumpHeader.offset < 0) {
        throw Errors::InvalidBody(
          Enums::Lump::GameLump,
          std::format("Static prop game lump header has a negative offset ({})", lumpHeader.offset)
        );
      }

      if (lumpHeader.length < 0) {
        throw Errors::InvalidBody(
          Enums::Lump::GameLump,
          std::format("Static prop game lump header has a negative length ({})", lumpHeader.length)
        );
      }

      if (lumpHeader.offset + lumpHeader.length > data.size_bytes()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::GameLump,
          std::format(
            "Static prop game lump header has offset + length ({}) overrunning the file ({})",
            lumpHeader.offset + lumpHeader.length,
            data.size_bytes()
          )
        );
      }

      const auto dictionaryData = OffsetDataView(std::span(&data[lumpHeader.offset], lumpHeader.length));
      const auto numDictionaryEntries = dictionaryData.parseStruct<int32_t>(
        0, "Static prop game lump length is shorter than a single int32 for the dictionary count"
      );
      staticPropDictionary = dictionaryData.parseStructArray<Structs::StaticPropDict>(
        sizeof(int32_t), numDictionaryEntries, "Static prop game lump dictionary entries overflowed the lump"
      );

      const auto leafData =
        dictionaryData.withRelativeOffset(sizeof(int32_t) + numDictionaryEntries * sizeof(Structs::StaticPropDict));
      const auto numLeaves = leafData.parseStruct<int32_t>(
        0, "Static prop game lump length is shorter than its dictionary entries plus a single int32 for the leaf count"
      );
      staticPropLeaves = leafData.parseStructArray<Structs::StaticPropLeaf>(
        sizeof(int32_t), numLeaves, "Static prop game lump leaves overflowed the lump"
      );

      const auto propData = leafData.withRelativeOffset(sizeof(int32_t) + numLeaves * sizeof(Structs::StaticPropLeaf));
      const auto numProps = propData.parseStruct<int32_t>(
        0, "Static prop game lump length is shorter than its dictionary, leaves, and a single int32 for the prop count"
      );
      const auto props = propData.parseStructArray<StaticProp>(
        sizeof(int32_t), numProps, "Static prop game lump props overflowed the lump"
      );

      return props;
    }

    [[nodiscard]] std::vector<Zip::ZipFileEntry> parsePakfileLump() const;

    void assertLumpHeaderValid(Enums::Lump lump, const Structs::Lump& lumpHeader) const;
  };
}
