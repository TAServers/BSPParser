#pragma once

#include "enums/surface.hpp"
#include "structs/common.hpp"
#include <span>
#include <string>
#include <vector>

namespace BspParser {
  class Bsp {
  public:
    struct Vertex {
      Structs::Vector position;
      Structs::Vector normal;
      Structs::Vector4 tangent;
      Structs::Vector2 uv;
    };

    struct Face {};

    struct Texture {
      Enums::Surface flags = Enums::Surface::None;
      Structs::Vector reflectivity;
      std::string path;

      int32_t width = 0;
      int32_t height = 0;
    };

    struct StaticProp {
      Structs::Vector position;
      Structs::EulerRotation angle;

      std::string model;
      int32_t skin;
    };

    explicit Bsp(std::span<const std::byte> data);

    [[nodiscard]] const std::vector<Texture>& getTextures() const;

    [[nodiscard]] const std::vector<StaticProp>& getStaticProps() const;

  private:
    std::span<const Structs::Vector> vertices;

    std::vector<Texture> textures;
    std::vector<StaticProp> staticProps;
  };
}
