#pragma once

#include "geom/GeomPoint.h"

#include <optional>

namespace aurora::geom {

class GeomBox {
 public:
  constexpr GeomBox() = default;
  constexpr GeomBox(DbUnit left, DbUnit bottom, DbUnit right, DbUnit top)
      : left_(left), bottom_(bottom), right_(right), top_(top) {
    normalize();
  }

  constexpr GeomBox(GeomPoint lowerLeft, GeomPoint upperRight)
      : GeomBox(lowerLeft.x, lowerLeft.y, upperRight.x, upperRight.y) {}

  [[nodiscard]] constexpr DbUnit left() const { return left_; }
  [[nodiscard]] constexpr DbUnit bottom() const { return bottom_; }
  [[nodiscard]] constexpr DbUnit right() const { return right_; }
  [[nodiscard]] constexpr DbUnit top() const { return top_; }
  [[nodiscard]] constexpr DbUnit width() const { return right_ - left_; }
  [[nodiscard]] constexpr DbUnit height() const { return top_ - bottom_; }
  [[nodiscard]] constexpr bool empty() const { return width() == 0 || height() == 0; }

  [[nodiscard]] constexpr GeomPoint lowerLeft() const { return {left_, bottom_}; }
  [[nodiscard]] constexpr GeomPoint upperRight() const { return {right_, top_}; }

  [[nodiscard]] constexpr bool contains(GeomPoint point) const {
    return point.x >= left_ && point.x <= right_ && point.y >= bottom_ && point.y <= top_;
  }

  [[nodiscard]] constexpr bool intersects(const GeomBox& other) const {
    return !(other.left_ > right_ || other.right_ < left_ || other.bottom_ > top_ ||
             other.top_ < bottom_);
  }

  [[nodiscard]] std::optional<GeomBox> intersection(const GeomBox& other) const;

  constexpr void translate(DbUnit dx, DbUnit dy) {
    left_ += dx;
    right_ += dx;
    bottom_ += dy;
    top_ += dy;
  }

 private:
  constexpr void normalize() {
    if (left_ > right_) {
      const auto tmp = left_;
      left_ = right_;
      right_ = tmp;
    }
    if (bottom_ > top_) {
      const auto tmp = bottom_;
      bottom_ = top_;
      top_ = tmp;
    }
  }

  DbUnit left_{0};
  DbUnit bottom_{0};
  DbUnit right_{0};
  DbUnit top_{0};
};

}  // namespace aurora::geom
