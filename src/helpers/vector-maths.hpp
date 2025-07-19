#pragma once

#include "../structs/common.hpp"
#include <cmath>

namespace BspParser {
  inline Structs::Vector xyz(const Structs::Vector4& v) {
    return {.x = v.x, .y = v.y, .z = v.z};
  }

  inline Structs::Vector add(const Structs::Vector& a, const Structs::Vector& b) {
    return Structs::Vector{
      .x = a.x + b.x,
      .y = a.y + b.y,
      .z = a.z + b.z,
    };
  }

  inline Structs::Vector sub(const Structs::Vector& a, const Structs::Vector& b) {
    return Structs::Vector{
      .x = a.x - b.x,
      .y = a.y - b.y,
      .z = a.z - b.z,
    };
  }

  inline Structs::Vector mul(const Structs::Vector& a, const float b) {
    return Structs::Vector{
      .x = a.x * b,
      .y = a.y * b,
      .z = a.z * b,
    };
  }

  inline Structs::Vector div(const Structs::Vector& a, const float b) {
    return Structs::Vector{
      .x = a.x / b,
      .y = a.y / b,
      .z = a.z / b,
    };
  }

  inline float dot(const Structs::Vector& a, const Structs::Vector& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

  inline Structs::Vector cross(const Structs::Vector& a, const Structs::Vector& b) {
    return Structs::Vector{
      .x = a.y * b.z - a.z * b.y,
      .y = a.z * b.x - a.x * b.z,
      .z = a.x * b.y - a.y * b.x,
    };
  }

  inline float length(const Structs::Vector& v) {
    return std::sqrt(dot(v, v));
  }

  inline Structs::Vector normalise(const Structs::Vector& v) {
    return div(v, length(v));
  }
}
