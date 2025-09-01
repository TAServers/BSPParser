#pragma once

// Doc comments for namespaces are required for Doxygen to register them

/**
 * Root namespace containing all BspParser classes and functions.
 */
namespace BspParser {
  /**
   * Accessor helpers for iterating contents of the BSP.
   */
  namespace Accessors {}

  /**
   * File format enums.
   */
  namespace Enums {}

  /**
   * File format structs.
   */
  namespace Structs {}

  /**
   * Zip file parsing.
   */
  namespace Zip {}
}

#include "./src/accessors/face-accessors.hpp"
#include "./src/accessors/prop-accessors.hpp"
#include "./src/accessors/texture-accessors.hpp"
#include "./src/bsp.hpp"
