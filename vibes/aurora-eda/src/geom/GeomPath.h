#pragma once

#include "geom/GeomPoint.h"

#include <span>
#include <vector>

namespace aurora::geom {

class GeomPath {
 public:
  GeomPath() = default;
  GeomPath(std::vector<GeomPoint> points, DbUnit width);

  [[nodiscard]] std::span<const GeomPoint> points() const;
  [[nodiscard]] DbUnit width() const;
  [[nodiscard]] bool empty() const;

  void setWidth(DbUnit width);
  void addPoint(GeomPoint point);
  void translate(DbUnit dx, DbUnit dy);

 private:
  std::vector<GeomPoint> points_;
  DbUnit width_{0};
};

}  // namespace aurora::geom
