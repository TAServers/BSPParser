#pragma once

#include <cstdint>
#include <stdexcept>

struct BSPTexture {
  BSPEnums::SURF flags = BSPEnums::SURF::NONE;
  BSPStructs::Vector reflectivity{0, 0, 0};
  const char* path = nullptr;
  int32_t width = 0, height = 0;
};

struct BSPStaticProp {
  BSPStructs::Vector pos{};
  BSPStructs::QAngle ang{};
  const char* model = nullptr;
  int32_t skin = 0;
};

class BSPMap {
  template <class StaticProp>
  BSPStaticProp GetStaticPropInternal(const int32_t index, const StaticProp* pStaticProps) const {
    if (index < 0 || index >= mNumStaticProps) throw std::out_of_range("Static prop index out of bounds");

    uint16_t dictIdx = pStaticProps[index].propType;
    if (dictIdx >= mNumStaticPropDictEntries) throw std::out_of_range("Static prop dictionary index out of bounds");

    BSPStaticProp prop;
    prop.pos = pStaticProps[index].origin;
    prop.ang = pStaticProps[index].angles;
    prop.model = mpStaticPropDict[dictIdx].modelName;
    prop.skin = pStaticProps[index].skin;

    return prop;
  }
};
