#pragma once

#include <exception>
#include "FileFormat/Enums.h"

namespace BSPErrors {
	class TriangulationError : public std::exception {
	public:
		TriangulationError(const char* message);
	};
}
