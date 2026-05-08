#pragma once

#include "geom/GeomBox.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"
#include "geom/GeomPolygon.h"

#include <optional>
#include <vector>

namespace aurora::geom {

[[nodiscard]] DbUnit snapCoordinate(DbUnit value, DbUnit grid);
[[nodiscard]] GeomPoint snapToGrid(GeomPoint point, DbUnit grid);
[[nodiscard]] GeomBox snapToGrid(const GeomBox& box, DbUnit grid);
[[nodiscard]] GeomPolygon snapToGrid(const GeomPolygon& polygon, DbUnit grid);
[[nodiscard]] GeomPath snapToGrid(const GeomPath& path, DbUnit grid);

[[nodiscard]] bool isManhattan(const GeomPolygon& polygon);
[[nodiscard]] DbUnit manhattanDistance(const GeomBox& lhs, const GeomBox& rhs);

[[nodiscard]] std::optional<GeomBox> boxIntersection(const GeomBox& lhs, const GeomBox& rhs);
[[nodiscard]] std::vector<GeomBox> boxUnion(const GeomBox& lhs, const GeomBox& rhs);
[[nodiscard]] std::vector<GeomBox> boxDifference(const GeomBox& subject, const GeomBox& cutter);

[[nodiscard]] bool meetsMinimumWidth(const GeomBox& box, DbUnit minWidth);
[[nodiscard]] bool meetsMinimumSpacing(const GeomBox& lhs, const GeomBox& rhs,
                                       DbUnit minSpacing);
[[nodiscard]] bool hasEnclosure(const GeomBox& outer, const GeomBox& inner, DbUnit enclosure);

}  // namespace aurora::geom
