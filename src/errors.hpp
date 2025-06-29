#pragma once

#include "enums/lump.hpp"
#include <cstdint>
#include <stdexcept>

#define ERROR_FOR_REASON(reason) \
  class reason final : public Error { \
  public: \
    explicit reason(const Enums::Lump lump, const std::string& message) : Error(Reason::reason, lump, message) {} \
  };

namespace BspParser::Errors {
  enum class Reason : uint8_t {
    InvalidHeader,
    InvalidBody,
    InvalidChecksum,
    UnsupportedVersion,
    OutOfBoundsAccess,
  };

  class Error : public std::runtime_error {
  public:
    Error(const Reason reason, const Enums::Lump lump, const std::string& message) :
      std::runtime_error(message), reason(reason), lump(lump) {}

    [[nodiscard]] Reason getReason() const {
      return reason;
    }

    [[nodiscard]] Enums::Lump getLump() const {
      return lump;
    }

  private:
    Reason reason;
    Enums::Lump lump;
  };

  ERROR_FOR_REASON(InvalidHeader);
  ERROR_FOR_REASON(InvalidBody);
  ERROR_FOR_REASON(InvalidChecksum);
  ERROR_FOR_REASON(UnsupportedVersion);
  ERROR_FOR_REASON(OutOfBoundsAccess);
}
