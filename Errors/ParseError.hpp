#pragma once
#include <stdexcept>
#include <string>
#include "FileFormat/Enums.h"

namespace BSPErrors {
	class ParseError : public std::runtime_error {
	public:
		ParseError(const char* message, BSPEnums::LUMP lump);

		BSPEnums::LUMP lump;
	};
}
