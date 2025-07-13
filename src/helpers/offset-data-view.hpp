#pragma once

#include "check-bounds.hpp"
#include <memory>
#include <span>

namespace BspParser {
  class OffsetDataView {
  public:
    template <typename T> using ValueOffsetPair = std::pair<T, size_t>;

    explicit OffsetDataView(std::span<const std::byte> data);

    explicit OffsetDataView(const OffsetDataView& from, size_t newOffset);

    ~OffsetDataView() = default;
    OffsetDataView(const OffsetDataView&) = delete;
    OffsetDataView& operator=(const OffsetDataView&) = delete;
    OffsetDataView(const OffsetDataView&&) = delete;
    OffsetDataView& operator=(const OffsetDataView&&) = delete;

    [[nodiscard]] OffsetDataView withRelativeOffset(size_t newOffset) const;

    template <typename T>
    [[nodiscard]] const T& parseStruct(const size_t relativeOffset, const char* errorMessage) const {
      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T), data.size_bytes(), errorMessage);

      return *reinterpret_cast<const T*>(&data[absoluteOffset]);
    }

    template <typename T>
    [[nodiscard]] std::span<const T> parseStructArray(
      const size_t relativeOffset, const size_t count, const char* errorMessage
    ) const {
      if (count == 0) {
        return std::span<const T>(static_cast<const T*>(nullptr), 0);
      }

      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T) * count, data.size_bytes(), errorMessage);

      const T* first = reinterpret_cast<const T*>(&data[absoluteOffset]);
      return std::span<const T>(first, count);
    }

    std::string parseString(size_t relativeOffset, const char* errorMessage) const;

  private:
    const std::span<const std::byte> data;
    const size_t offset;
  };
}
