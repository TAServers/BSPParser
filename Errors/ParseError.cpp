#include "ParseError.hpp"

BSPErrors::ParseError::ParseError(const char* const message, BSPEnums::LUMP lump) : std::runtime_error(message), lump(lump) {}
