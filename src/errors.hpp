#pragma once

#include <cstdint>
#include <stdexcept>

#define ERROR_FOR_REASON(reason) \
  class reason : public Error { \
  public: \
    explicit reason(const char* message) : Error(Reason::reason, message) {} \
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
    Error(const Reason reason, const char* message) : std::runtime_error(message), reason(reason) {}

    Reason getReason() const {
      return reason;
    }

  private:
    Reason reason;
  };

  ERROR_FOR_REASON(InvalidHeader);
  ERROR_FOR_REASON(InvalidBody);
  ERROR_FOR_REASON(InvalidChecksum);
  ERROR_FOR_REASON(UnsupportedVersion);
  ERROR_FOR_REASON(OutOfBoundsAccess);
}
