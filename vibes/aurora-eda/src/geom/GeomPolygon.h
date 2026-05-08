#pragma once

#include "geom/GeomBox.h"
#include "geom/GeomPoint.h"

#include <span>
#include <vector>

namespace aurora::geom {

class GeomPolygon {
 public:
  GeomPolygon() = default;
  explicit GeomPolygon(std::vector<GeomPoint> points);

  [[nodiscard]] std::span<const GeomPoint> points() const;
  [[nodiscard]] bool empty() const;
  [[nodiscard]] GeomBox boundingBox() const;

  void addPoint(GeomPoint point);
  void translate(DbUnit dx, DbUnit dy);

 private:
  std::vector<GeomPoint> points_;
};

}  // namespace aurora::geom
