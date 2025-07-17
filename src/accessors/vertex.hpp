#pragma once

#include "../structs/common.hpp"

namespace BspParser::Accessors {
  struct Vertex {
    Structs::Vector position;
    Structs::Vector normal;
    Structs::Vector4 tangent;
    Structs::Vector2 uv;
  };
}
