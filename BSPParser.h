#pragma once

#include <cstdint>
#include <stdexcept>

struct BSPTexture {
  BSPEnums::SURF flags = BSPEnums::SURF::NONE;
  BSPStructs::Vector reflectivity{0, 0, 0};
  const char* path = nullptr;
  int32_t width = 0, height = 0;
};

class BSPMap {};
