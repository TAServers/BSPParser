#include "triangulated-displacement.hpp"
#include "../helpers/calculate-tangent.hpp"
#include "../helpers/vector-maths.hpp"

namespace BspParser {
  using namespace Internal;

  void TriangulatedDisplacement::generateInternalNormals() {
    for (size_t y = 0; y < numVerticesPerAxis; y++) {
      for (size_t x = 0; x < numVerticesPerAxis; x++) {
        auto& vertex = vertices[y * numVerticesPerAxis + x];

        vertex.normal = generateInternalNormal(x, y);
        vertex.tangent = calculateTangent(vertex.normal, textureInfo);
      }
    }
  }

  Structs::Vector TriangulatedDisplacement::generateInternalNormal(const size_t x, const size_t y) const {
    const auto centre = getVertex(x, y).position;

    auto normal = Structs::Vector{};

    // All cross products must be **clockwise**
    // The normal is taken for the entire quadrant to avoid seams along triangulated edges
    if (x > 0) {
      const auto& left = getVertex(x - 1, y).position;

      if (y > 0) {
        const auto& bottom = getVertex(x, y - 1).position;
        const auto& bottomLeft = getVertex(x - 1, y - 1).position;

        normal = add(normal, cross(sub(left, centre), sub(bottom, centre)));

        // Bottom is right, left is top (relative to bottomLeft)
        normal = add(normal, cross(sub(bottom, bottomLeft), sub(left, bottomLeft)));
      }

      if (y < numVerticesPerAxis - 1) {
        const auto& top = getVertex(x, y + 1).position;
        const auto& topLeft = getVertex(x - 1, y + 1).position;

        normal = add(normal, cross(sub(top, centre), sub(left, centre)));

        // Top is right, left is bottom (relative to topLeft)
        normal = add(normal, cross(sub(left, topLeft), sub(top, topLeft)));
      }
    }

    if (x < numVerticesPerAxis - 1) {
      const auto& right = getVertex(x + 1, y).position;

      if (y > 0) {
        const auto& bottom = getVertex(x, y - 1).position;
        const auto& bottomRight = getVertex(x + 1, y - 1).position;

        normal = add(normal, cross(sub(bottom, centre), sub(right, centre)));

        // Bottom is left, right is top (relative to bottomBottom)
        normal = add(normal, cross(sub(right, bottomRight), sub(bottom, bottomRight)));
      }

      if (y < numVerticesPerAxis - 1) {
        const auto& top = getVertex(x, y + 1).position;
        const auto& topRight = getVertex(x + 1, y + 1).position;

        normal = add(normal, cross(sub(right, centre), sub(top, centre)));

        // Top is left, right is bottom (relative to topLeft)
        normal = add(normal, cross(sub(top, topRight), sub(right, topRight)));
      }
    }

    return normalise(normal);
  }
}
