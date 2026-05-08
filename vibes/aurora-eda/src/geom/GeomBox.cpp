#include "geom/GeomBox.h"

#include <algorithm>

namespace aurora::geom {

std::optional<GeomBox> GeomBox::intersection(const GeomBox& other) const {
  if (!intersects(other)) {
    return std::nullopt;
  }

  return GeomBox{
      std::max(left(), other.left()),
      std::max(bottom(), other.bottom()),
      std::min(right(), other.right()),
      std::min(top(), other.top()),
  };
}

}  // namespace aurora::geom
