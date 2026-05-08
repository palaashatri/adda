#pragma once

#include <cstdint>

namespace aurora::geom {

using DbUnit = std::int64_t;

struct GeomPoint {
  DbUnit x{0};
  DbUnit y{0};

  constexpr GeomPoint() = default;
  constexpr GeomPoint(DbUnit xValue, DbUnit yValue) : x(xValue), y(yValue) {}

  [[nodiscard]] constexpr GeomPoint translated(DbUnit dx, DbUnit dy) const {
    return {x + dx, y + dy};
  }
};

[[nodiscard]] constexpr bool operator==(const GeomPoint& lhs, const GeomPoint& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

[[nodiscard]] constexpr bool operator!=(const GeomPoint& lhs, const GeomPoint& rhs) {
  return !(lhs == rhs);
}

}  // namespace aurora::geom
