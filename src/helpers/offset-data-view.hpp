#pragma once

#include "check-bounds.hpp"
#include <memory>
#include <vector>

namespace BspParser {
  class OffsetDataView {
  public:
    template <typename T> using ValueOffsetPair = std::pair<T, size_t>;

    explicit OffsetDataView(const std::weak_ptr<std::vector<std::byte>>& data);

    explicit OffsetDataView(const OffsetDataView& from, const size_t newOffset);

    ~OffsetDataView() = default;
    OffsetDataView(const OffsetDataView&) = delete;
    OffsetDataView& operator=(const OffsetDataView&) = delete;
    OffsetDataView(const OffsetDataView&&) = delete;
    OffsetDataView& operator=(const OffsetDataView&&) = delete;

    [[nodiscard]] OffsetDataView withOffset(const size_t newOffset) const;

    template <typename T>
    [[nodiscard]] ValueOffsetPair<T> parseStruct(const size_t relativeOffset, const char* errorMessage) const {
      const auto lockedData = getLockedData();
      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T), lockedData->size(), errorMessage);

      return std::make_pair(*reinterpret_cast<const T*>(&lockedData->at(absoluteOffset)), absoluteOffset);
    }

    template <typename T>
    [[nodiscard]] std::vector<ValueOffsetPair<T>> parseStructArray(
      const size_t relativeOffset, const size_t count, const char* errorMessage
    ) const {
      const auto lockedData = getLockedData();
      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T) * count, lockedData->size(), errorMessage);

      std::vector<ValueOffsetPair<T>> parsed;
      parsed.reserve(count);

      for (size_t i = 0; i < count; i++) {
        const auto currentOffset = absoluteOffset + sizeof(T) * i;
        parsed.emplace_back(*reinterpret_cast<const T*>(&lockedData->at(currentOffset)), currentOffset);
      }

      return std::move(parsed);
    }

    template <typename T>
    [[nodiscard]] std::vector<T> parseStructArrayWithoutOffsets(
      const size_t relativeOffset, const size_t count, const char* errorMessage
    ) const {
      const auto lockedData = getLockedData();
      const auto absoluteOffset = offset + relativeOffset;
      checkBounds(absoluteOffset, sizeof(T) * count, lockedData->size(), errorMessage);

      const T* first = reinterpret_cast<const T*>(&lockedData->at(absoluteOffset));
      return std::vector<T>(first, first + count);
    }

    std::string parseString(const size_t relativeOffset, const char* errorMessage) const;

  private:
    const std::weak_ptr<std::vector<std::byte>> data;
    const size_t offset;

    [[nodiscard]] std::shared_ptr<std::vector<std::byte>> getLockedData() const;
  };
}
