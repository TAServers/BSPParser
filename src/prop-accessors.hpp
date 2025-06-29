#pragma once

#include "bsp.hpp"
#include <functional>

namespace BspParser::Accessors {
  template <typename Visitor>
  concept StaticPropIteratee = requires(Visitor visitor) {
    { visitor(std::declval<const Structs::StaticPropV4&>(), std::declval<const char*>()) };
    { visitor(std::declval<const Structs::StaticPropV5&>(), std::declval<const char*>()) };
    { visitor(std::declval<const Structs::StaticPropV6&>(), std::declval<const char*>()) };
    { visitor(std::declval<const Structs::StaticPropV7Multiplayer2013&>(), std::declval<const char*>()) };
  };

  template <StaticPropIteratee Iteratee> void iterateStaticProps(const Bsp& bsp, Iteratee iteratee) {
    if (!bsp.staticProps.has_value() || !bsp.staticPropDictionary.has_value()) {
      return;
    }

    std::visit(
      [&bsp, &iteratee](const auto props) {
        for (const auto& prop : props) {
          const auto dictionaryIndex = prop.propType;
          if (dictionaryIndex >= bsp.staticPropDictionary->size()) {
            // TODO: Generate warning for out of range dictionary indices
            continue;
          }

          const auto* const modelPath = bsp.staticPropDictionary.value()[dictionaryIndex].modelName.data();
          iteratee(prop, modelPath);
        }
      },
      bsp.staticProps.value()
    );
  }
}
