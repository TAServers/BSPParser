#include "TriangulationError.hpp"

BSPErrors::TriangulationError::TriangulationError(const char* const message) : std::runtime_error(message) {}