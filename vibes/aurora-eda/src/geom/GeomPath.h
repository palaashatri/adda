#pragma once

#include "geom/GeomPoint.h"

#include <span>
#include <vector>

namespace aurora::geom {

enum class PathCornerStyle { Miter, Round, Square };

class GeomPath {
 public:
  GeomPath() = default;
  GeomPath(std::vector<GeomPoint> points, DbUnit width,
           PathCornerStyle cornerStyle = PathCornerStyle::Miter);

  [[nodiscard]] std::span<const GeomPoint> points() const;
  [[nodiscard]] DbUnit width() const;
  [[nodiscard]] PathCornerStyle cornerStyle() const;
  [[nodiscard]] bool empty() const;

  void setWidth(DbUnit width);
  void setCornerStyle(PathCornerStyle style);
  void addPoint(GeomPoint point);
  void translate(DbUnit dx, DbUnit dy);

 private:
  std::vector<GeomPoint> points_;
  DbUnit width_{0};
  PathCornerStyle cornerStyle_{PathCornerStyle::Miter};
};

}  // namespace aurora::geom
