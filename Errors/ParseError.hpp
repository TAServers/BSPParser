#pragma once

#include <exception>
#include <string>
#include "FileFormat/Enums.h"

namespace BSPErrors {
	class ParseError : public std::exception {
	public:
		ParseError(const char* message, BSPEnums::LUMP lump);

		BSPEnums::LUMP lump;
	};
}
