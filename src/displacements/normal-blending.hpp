#pragma once

#include "triangulated-displacement.hpp"

namespace BspParser::Internal {
  /**
   * @remarks Largely copied from VRAD in the Source Engine 2013 SDK, with some cleanup.
   * @param displacements All displacements in the BSP. Indices must match the underlying displacement infos.
   */
  void blendNeighbouringDisplacementNormals(std::span<TriangulatedDisplacement> displacements);
}
