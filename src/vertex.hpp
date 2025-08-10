#pragma once

#include "structs/common.hpp"

namespace BspParser {
  struct Vertex {
    Structs::Vector position;
    Structs::Vector normal;
    Structs::Vector4 tangent;
    Structs::Vector2 uv;
    float alpha = 0.f;
  };
}
