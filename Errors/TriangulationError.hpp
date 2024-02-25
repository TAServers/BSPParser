#pragma once

#include <stdexcept>
#include "FileFormat/Enums.h"

namespace BSPErrors {
	class TriangulationError : public std::runtime_error {
	public:
		TriangulationError(const char* message);
	};
}
