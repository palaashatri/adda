#include "geom/GeomPolygon.h"

#include <algorithm>
#include <stdexcept>

namespace aurora::geom {

GeomPolygon::GeomPolygon(std::vector<GeomPoint> points) : points_(std::move(points)) {}

std::span<const GeomPoint> GeomPolygon::points() const {
  return points_;
}

bool GeomPolygon::empty() const {
  return points_.empty();
}

GeomBox GeomPolygon::boundingBox() const {
  if (points_.empty()) {
    throw std::logic_error("Cannot compute a bounding box for an empty polygon");
  }

  auto minX = points_.front().x;
  auto maxX = points_.front().x;
  auto minY = points_.front().y;
  auto maxY = points_.front().y;

  for (const auto& point : points_) {
    minX = std::min(minX, point.x);
    maxX = std::max(maxX, point.x);
    minY = std::min(minY, point.y);
    maxY = std::max(maxY, point.y);
  }

  return {minX, minY, maxX, maxY};
}

void GeomPolygon::addPoint(GeomPoint point) {
  points_.push_back(point);
}

void GeomPolygon::translate(DbUnit dx, DbUnit dy) {
  for (auto& point : points_) {
    point = point.translated(dx, dy);
  }
}

}  // namespace aurora::geom
