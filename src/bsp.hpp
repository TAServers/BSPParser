#pragma once

#include "structs/common.hpp"
#include "structs/detail-props.hpp"
#include "structs/displacements.hpp"
#include "structs/geometry.hpp"
#include "structs/headers.hpp"
#include "structs/models.hpp"
#include "structs/static-props.hpp"
#include "structs/textures.hpp"
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace BspParser {
  struct Bsp {
    explicit Bsp(std::span<const std::byte> data);

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

    std::span<const Structs::DetailObjectDict> detailObjectDictionary;
    std::span<const Structs::DetailObject> detailObjects;

    std::span<const Structs::StaticPropDict> staticPropDictionary;
    std::span<const Structs::StaticPropLeaf> staticPropLeaves;

    std::variant<
      std::span<const Structs::StaticPropV4>,
      std::span<const Structs::StaticPropV5>,
      std::span<const Structs::StaticPropV6>,
      std::span<const Structs::StaticPropV7Multiplayer2013>>
      staticProps;
  };
}
