#include "geom/GeomPath.h"

#include <stdexcept>

namespace aurora::geom {

GeomPath::GeomPath(std::vector<GeomPoint> points, DbUnit width,
                   PathCornerStyle cornerStyle)
    : points_(std::move(points)), width_(width), cornerStyle_(cornerStyle) {
  if (width < 0) {
    throw std::invalid_argument("Path width must be non-negative");
  }
}

std::span<const GeomPoint> GeomPath::points() const {
  return points_;
}

DbUnit GeomPath::width() const {
  return width_;
}

PathCornerStyle GeomPath::cornerStyle() const {
  return cornerStyle_;
}

bool GeomPath::empty() const {
  return points_.empty();
}

void GeomPath::setWidth(DbUnit width) {
  if (width < 0) {
    throw std::invalid_argument("Path width must be non-negative");
  }
  width_ = width;
}

void GeomPath::setCornerStyle(PathCornerStyle style) {
  cornerStyle_ = style;
}

void GeomPath::addPoint(GeomPoint point) {
  points_.push_back(point);
}

void GeomPath::translate(DbUnit dx, DbUnit dy) {
  for (auto& point : points_) {
    point = point.translated(dx, dy);
  }
}

}  // namespace aurora::geom
