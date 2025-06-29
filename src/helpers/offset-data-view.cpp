#include "offset-data-view.hpp"

namespace BspParser {
  OffsetDataView::OffsetDataView(const std::span<const std::byte> data) : data(data), offset(0) {}

  OffsetDataView::OffsetDataView(const OffsetDataView& from, const size_t newOffset) :
    data(from.data), offset(newOffset) {}

  OffsetDataView OffsetDataView::withRelativeOffset(const size_t newOffset) const {
    return OffsetDataView(*this, offset + newOffset);
  }

  std::string OffsetDataView::parseString(const size_t relativeOffset, const char* errorMessage) const {
    const auto absoluteOffset = offset + relativeOffset;

    for (size_t i = absoluteOffset; i < data.size_bytes(); i++) {
      if (data[i] == static_cast<std::byte>(0)) {
        return reinterpret_cast<const char*>(&data[absoluteOffset]);
      }
    }

    throw Errors::OutOfBoundsAccess(Enums::Lump::None, errorMessage);
  }
}
