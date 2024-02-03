#include "ParseError.hpp"

BSPErrors::ParseError::ParseError(const char* const message, BSPEnums::LUMP lump) : std::exception(message), lump(lump) {}
