#include "texture-accessors.hpp"

namespace BspParser::Accessors {
  void iterateTextures(
    const Bsp& bsp, const std::function<void(const Structs::TexData& texture, const char* path)>& iteratee
  ) {
    for (const auto& texture : bsp.textureDatas) {
      if (texture.nameStringTableId < 0 || texture.nameStringTableId >= bsp.textureStringTable.size()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::TextureData,
          std::format(
            "Texture data entry string table ID '{}' is out of bounds of the string table lump",
            texture.nameStringTableId
          )
        );
      }

      const auto stringId = bsp.textureStringTable[texture.nameStringTableId];
      if (stringId < 0 || stringId >= bsp.textureStringData.size_bytes()) {
        throw Errors::OutOfBoundsAccess(
          Enums::Lump::TextureDataStringTable,
          std::format("Texture string table offset '{}' is out of bounds of the string data lump", stringId)
        );
      }

      const auto* const path = &bsp.textureStringData[stringId];

      iteratee(texture, path);
    }
  }
}
