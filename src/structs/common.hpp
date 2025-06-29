#pragma once

namespace BspParser::Structs {
  struct Vector2 {
    union {
      float x = 0;
      float u;
    };

    union {
      float y = 0;
      float v;
    };
  };

  struct Vector {
    float x = 0;
    float y = 0;
    float z = 0;
  };

  struct Vector4 {
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;
  };

  struct EulerRotation {
    float x = 0;
    float y = 0;
    float z = 0;
  };

  struct ColourRgbExp32 {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    int8_t exponent = 1;
  };
}
