#include "offset-data-view.hpp"

namespace BspParser {
  OffsetDataView::OffsetDataView(const std::weak_ptr<std::vector<std::byte>>& data) : data(data), offset(0) {}

  OffsetDataView::OffsetDataView(const OffsetDataView& from, const size_t newOffset) :
    data(from.data), offset(newOffset) {}

  OffsetDataView OffsetDataView::withOffset(const size_t newOffset) const {
    return OffsetDataView(*this, newOffset);
  }

  std::string OffsetDataView::parseString(const size_t relativeOffset, const char* errorMessage) const {
    const auto lockedData = getLockedData();
    const auto absoluteOffset = offset + relativeOffset;

    for (size_t i = absoluteOffset; i < lockedData->size(); i++) {
      if (lockedData->at(i) == std::byte(0)) {
        return reinterpret_cast<const char*>(&lockedData->at(absoluteOffset));
      }
    }

    throw OutOfBoundsAccess(errorMessage);
  }

  std::shared_ptr<std::vector<std::byte>> OffsetDataView::getLockedData() const {
    if (data.expired()) {
      throw std::runtime_error("Attempted to lock an expired weak_ptr to underlying data");
    }

    return data.lock();
  }
}
