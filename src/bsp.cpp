#include "bsp.hpp"
#include "displacements/normal-blending.hpp"
#include "structs/physics.hpp"

namespace BspParser {
  Bsp::Bsp(const std::span<std::byte const> data) : data(data) {
    if (data.size_bytes() < sizeof(Structs::Header)) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::None,
        std::format(
          "Size of data ({}) is less than the size of the header type ({})", data.size_bytes(), sizeof(Structs::Header)
        )
      );
    }

    // BSP parser takes no ownership of the data to avoid unnecessary copies
    // Will need to change if/when implementing lump decompression
    // ReSharper disable once CppDFALocalValueEscapesFunction
    header = reinterpret_cast<const Structs::Header*>(data.data());

    if (header->identifier != Structs::IDBSP_HEADER) {
      throw Errors::InvalidHeader(Enums::Lump::None, "Header's identifier is not 'VBSP'");
    }

    if (header->version < 19 || header->version > 21) {
      throw Errors::UnsupportedVersion(Enums::Lump::None, std::format("Unsupported BSP version {}", header->version));
    }

    gameLumps = parseGameLumpHeaders();

    vertices = parseLump<Structs::Vector>(Enums::Lump::Vertices, Limits::MAX_MAP_VERTS);
    planes = parseLump<Structs::Plane>(Enums::Lump::Planes, Limits::MAX_MAP_PLANES);
    edges = parseLump<Structs::Edge>(Enums::Lump::Edges, Limits::MAX_MAP_EDGES);
    surfaceEdges = parseLump<int32_t>(Enums::Lump::SurfaceEdges, Limits::MAX_MAP_SURFEDGES);
    faces = parseLump<Structs::Face>(Enums::Lump::Faces, Limits::MAX_MAP_FACES);

    textureInfos = parseLump<Structs::TexInfo>(Enums::Lump::TextureInfo, Limits::MAX_MAP_TEXINFO);
    textureDatas = parseLump<Structs::TexData>(Enums::Lump::TextureData, Limits::MAX_MAP_TEXDATA);
    textureStringTable = parseLump<int32_t>(Enums::Lump::TextureDataStringTable, Limits::MAX_MAP_TEXDATA_STRING_TABLE);
    textureStringData = parseLump<char>(Enums::Lump::TextureDataStringData, Limits::MAX_MAP_TEXDATA_STRING_DATA);

    models = parseLump<Structs::Model>(Enums::Lump::Models, Limits::MAX_MAP_MODELS);

    displacementInfos = parseLump<Structs::DispInfo>(Enums::Lump::DisplacementInfo, Limits::MAX_MAP_DISPINFO);
    displacementVertices = parseLump<Structs::DispVert>(Enums::Lump::DisplacementVertices, Limits::MAX_MAP_DISP_VERTS);

    displacements.reserve(displacementInfos.size());
    for (const auto& displacementInfo : displacementInfos) {
      displacements.push_back(createTriangulatedDisplacement(displacementInfo));
    }
    Internal::blendNeighbouringDisplacementNormals(displacements);

    physicsModels = parsePhysCollideLump();

    compressedPakfile = parsePakfileLump();

    for (const auto& gameLump : gameLumps) {
      switch (gameLump.id) {
        case Enums::GameLumpID::DetailProps:
          // TODO: Implement parsing for detail props
          break;
        case Enums::GameLumpID::StaticProps:
          switch (gameLump.version) {
            case 4:
              staticProps = parseStaticPropLump<Structs::StaticPropV4>(gameLump);
              break;
            case 5:
              staticProps = parseStaticPropLump<Structs::StaticPropV5>(gameLump);
              break;
            case 6:
              staticProps = parseStaticPropLump<Structs::StaticPropV6>(gameLump);
              break;

            // A non-standard version 7 static prop lump exists in the 2013 multiplayer SDK exclusively
            // This may appear as either version 7 or 10, but is not compatible with other engine versions' v7 or v10 (thank you Valve)
            case 7:
            case 10:
              staticProps = parseStaticPropLump<Structs::StaticPropV7Multiplayer2013>(gameLump);
              break;

            default:
              // TODO: Raise a warning for unsupported static prop lumps (we don't need to error if the rest of the file parsed)
              break;
          }
          break;
        default:
          break;
      }
    }
  }

  std::span<const Structs::GameLump> Bsp::parseGameLumpHeaders() const {
    const auto& lumpHeader = header->lumps.at(static_cast<size_t>(Enums::Lump::GameLump));

    assertLumpHeaderValid(Enums::Lump::GameLump, lumpHeader);

    if (lumpHeader.length < sizeof(int32_t)) {
      throw Errors::InvalidBody(
        Enums::Lump::GameLump,
        std::format(
          "Game lump header has length ({}) less than the single int32 needed for the count", lumpHeader.length
        )
      );
    }

    const auto numGameLumpHeaders = *reinterpret_cast<const int32_t*>(&data[lumpHeader.offset]);

    const auto actualLumpSizeBytes = numGameLumpHeaders * sizeof(Structs::GameLump) + sizeof(int32_t);
    if (actualLumpSizeBytes > lumpHeader.length) {
      throw Errors::OutOfBoundsAccess(
        Enums::Lump::GameLump,
        std::format(
          "Actual size of game lump given by the number of entries ({}) exceeds the total size of the lump ({})",
          actualLumpSizeBytes,
          lumpHeader.length
        )
      );
    }

    return std::span(
      reinterpret_cast<const Structs::GameLump*>(&data[lumpHeader.offset + sizeof(int32_t)]), numGameLumpHeaders
    );
  }

  std::vector<Bsp::PhysModel> Bsp::parsePhysCollideLump() const {
    const auto& lumpHeader = header->lumps.at(static_cast<size_t>(Enums::Lump::PhysCollide));
    assertLumpHeaderValid(Enums::Lump::PhysCollide, lumpHeader);

    std::vector<PhysModel> physicsModels;

    size_t offset = lumpHeader.offset;
    while (true) {
      auto remainingBytes = lumpHeader.length - (offset - lumpHeader.offset);

      if (remainingBytes < sizeof(Structs::PhysModelHeader)) {
        throw Errors::InvalidBody(
          Enums::Lump::PhysCollide, std::format("PhysCollide lump is not terminated with a negative model index")
        );
      }

      const auto& modelHeader = *reinterpret_cast<const Structs::PhysModelHeader*>(&data[offset]);
      offset += sizeof(Structs::PhysModelHeader);
      remainingBytes -= sizeof(Structs::PhysModelHeader);

      if (modelHeader.modelIndex < 0) {
        break;
      }

      if (remainingBytes < modelHeader.collisionDataSize + modelHeader.textSectionSize) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::PhysCollide,
          std::format(
            "PhysCollide model header has size ({} + {}) exceeding the lump ({})",
            modelHeader.collisionDataSize,
            modelHeader.textSectionSize,
            remainingBytes
          )
        );
      }

      physicsModels.push_back(
        PhysModel{
          .modelIndex = modelHeader.modelIndex,
          .solidCount = modelHeader.solidCount,
          .collisionData = data.subspan(offset, modelHeader.collisionDataSize),
          .textSectionData = data.subspan(offset + modelHeader.collisionDataSize, modelHeader.textSectionSize),
        }
      );
      offset += modelHeader.collisionDataSize + modelHeader.textSectionSize;
    }

    return std::move(physicsModels);
  }

  std::vector<Zip::ZipFileEntry> Bsp::parsePakfileLump() const {
    const auto& lumpHeader = header->lumps.at(static_cast<size_t>(Enums::Lump::PakFile));
    assertLumpHeaderValid(Enums::Lump::PakFile, lumpHeader);

    const auto pakfileData = data.subspan(lumpHeader.offset, lumpHeader.length);

    return Zip::readZipFileEntries(pakfileData);
  }

  void Bsp::assertLumpHeaderValid(const Enums::Lump lump, const Structs::Lump& lumpHeader) const {
    if (lumpHeader.offset < 0) {
      throw Errors::InvalidBody(lump, std::format("Lump header has a negative offset ({})", lumpHeader.offset));
    }

    if (lumpHeader.length < 0) {
      throw Errors::InvalidBody(lump, std::format("Lump header has a negative length ({})", lumpHeader.length));
    }

    if (lumpHeader.offset + lumpHeader.length > data.size_bytes()) {
      throw Errors::OutOfBoundsAccess(
        lump,
        std::format(
          "Lump header has offset + length ({}) overrunning the file ({})",
          lumpHeader.offset + lumpHeader.length,
          data.size_bytes()
        )
      );
    }
  }

  TriangulatedDisplacement Bsp::createTriangulatedDisplacement(const Structs::DispInfo& displacementInfo) const {
    const auto& face = faces[displacementInfo.mapFace];
    const auto& textureInfo = textureInfos[face.texInfo];
    const auto& textureData = textureDatas[textureInfo.texData];
    const auto surfaceEdgesForDisplacement = surfaceEdges.subspan(face.firstEdge, face.numEdges);

    return TriangulatedDisplacement(
      displacementInfo, displacementVertices, edges, vertices, surfaceEdgesForDisplacement, textureInfo, textureData
    );
  }
}
