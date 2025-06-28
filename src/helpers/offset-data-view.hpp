#pragma once

#include "check-bounds.hpp"
#include <memory>
#include <span>
#include <vector>

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

    [[nodiscard]] OffsetDataView withOffset(size_t newOffset) const;

    template <typename T>
    [[nodiscard]] ValueOffsetPair<T> parseStruct(const size_t relativeOffset, const char* errorMessage) const {
      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T), data.size_bytes(), errorMessage);

      return std::make_pair(*reinterpret_cast<const T*>(&data[absoluteOffset]), absoluteOffset);
    }

    template <typename T>
    [[nodiscard]] std::vector<ValueOffsetPair<T>> parseStructArray(
      const size_t relativeOffset, const size_t count, const char* errorMessage
    ) const {
      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T) * count, data.size_bytes(), errorMessage);

      std::vector<ValueOffsetPair<T>> parsed;
      parsed.reserve(count);

      for (size_t i = 0; i < count; i++) {
        const auto currentOffset = absoluteOffset + sizeof(T) * i;
        parsed.emplace_back(*reinterpret_cast<const T*>(&data[currentOffset]), currentOffset);
      }

      return std::move(parsed);
    }

    template <typename T>
    [[nodiscard]] std::span<const T> parseStructArrayWithoutOffsets(
      const size_t relativeOffset, const size_t count, const char* errorMessage
    ) const {
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
