#include "geom/GeomOps.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace aurora::geom {
namespace {

[[nodiscard]] bool canMergeAsSingleBox(const GeomBox& lhs, const GeomBox& rhs) {
  const bool sameVerticalSpan = lhs.bottom() == rhs.bottom() && lhs.top() == rhs.top();
  const bool sameHorizontalSpan = lhs.left() == rhs.left() && lhs.right() == rhs.right();
  return (sameVerticalSpan && lhs.right() >= rhs.left() && rhs.right() >= lhs.left()) ||
         (sameHorizontalSpan && lhs.top() >= rhs.bottom() && rhs.top() >= lhs.bottom());
}

void appendIfNonEmpty(std::vector<GeomBox>& boxes, GeomBox box) {
  if (!box.empty()) {
    boxes.push_back(box);
  }
}

}  // namespace

DbUnit snapCoordinate(DbUnit value, DbUnit grid) {
  if (grid <= 0) {
    throw std::invalid_argument("Snap grid must be positive");
  }

  const auto remainder = value % grid;
  if (remainder == 0) {
    return value;
  }

  const auto lower = value - remainder;
  const auto upper = value > 0 ? lower + grid : lower - grid;
  return std::llabs(value - lower) <= std::llabs(upper - value) ? lower : upper;
}

GeomPoint snapToGrid(GeomPoint point, DbUnit grid) {
  return {snapCoordinate(point.x, grid), snapCoordinate(point.y, grid)};
}

GeomBox snapToGrid(const GeomBox& box, DbUnit grid) {
  return {snapCoordinate(box.left(), grid), snapCoordinate(box.bottom(), grid),
          snapCoordinate(box.right(), grid), snapCoordinate(box.top(), grid)};
}

GeomPolygon snapToGrid(const GeomPolygon& polygon, DbUnit grid) {
  std::vector<GeomPoint> points;
  points.reserve(polygon.points().size());
  for (const auto& point : polygon.points()) {
    points.push_back(snapToGrid(point, grid));
  }
  return GeomPolygon{std::move(points)};
}

GeomPath snapToGrid(const GeomPath& path, DbUnit grid) {
  std::vector<GeomPoint> points;
  points.reserve(path.points().size());
  for (const auto& point : path.points()) {
    points.push_back(snapToGrid(point, grid));
  }
  return GeomPath{std::move(points), snapCoordinate(path.width(), grid)};
}

bool isManhattan(const GeomPolygon& polygon) {
  const auto points = polygon.points();
  if (points.size() < 2) {
    return true;
  }

  for (std::size_t index = 0; index < points.size(); ++index) {
    const auto& current = points[index];
    const auto& next = points[(index + 1) % points.size()];
    if (current.x != next.x && current.y != next.y) {
      return false;
    }
  }
  return true;
}

DbUnit manhattanDistance(const GeomBox& lhs, const GeomBox& rhs) {
  const auto dx =
      std::max<DbUnit>({DbUnit{0}, rhs.left() - lhs.right(), lhs.left() - rhs.right()});
  const auto dy =
      std::max<DbUnit>({DbUnit{0}, rhs.bottom() - lhs.top(), lhs.bottom() - rhs.top()});
  return dx + dy;
}

std::optional<GeomBox> boxIntersection(const GeomBox& lhs, const GeomBox& rhs) {
  return lhs.intersection(rhs);
}

std::vector<GeomBox> boxUnion(const GeomBox& lhs, const GeomBox& rhs) {
  if (lhs.empty()) {
    return rhs.empty() ? std::vector<GeomBox>{} : std::vector<GeomBox>{rhs};
  }
  if (rhs.empty()) {
    return {lhs};
  }

  if (canMergeAsSingleBox(lhs, rhs)) {
    return {GeomBox{std::min(lhs.left(), rhs.left()), std::min(lhs.bottom(), rhs.bottom()),
                    std::max(lhs.right(), rhs.right()), std::max(lhs.top(), rhs.top())}};
  }

  return {lhs, rhs};
}

std::vector<GeomBox> boxDifference(const GeomBox& subject, const GeomBox& cutter) {
  if (subject.empty()) {
    return {};
  }

  const auto intersection = subject.intersection(cutter);
  if (!intersection || intersection->empty()) {
    return {subject};
  }

  std::vector<GeomBox> result;
  result.reserve(4);

  appendIfNonEmpty(result, {subject.left(), subject.bottom(), intersection->left(), subject.top()});
  appendIfNonEmpty(result, {intersection->right(), subject.bottom(), subject.right(), subject.top()});
  appendIfNonEmpty(result,
                   {intersection->left(), subject.bottom(), intersection->right(),
                    intersection->bottom()});
  appendIfNonEmpty(result,
                   {intersection->left(), intersection->top(), intersection->right(), subject.top()});

  return result;
}

bool meetsMinimumWidth(const GeomBox& box, DbUnit minWidth) {
  if (minWidth < 0) {
    throw std::invalid_argument("Minimum width must be non-negative");
  }
  return !box.empty() && std::min(box.width(), box.height()) >= minWidth;
}

bool meetsMinimumSpacing(const GeomBox& lhs, const GeomBox& rhs, DbUnit minSpacing) {
  if (minSpacing < 0) {
    throw std::invalid_argument("Minimum spacing must be non-negative");
  }
  return manhattanDistance(lhs, rhs) >= minSpacing;
}

bool hasEnclosure(const GeomBox& outer, const GeomBox& inner, DbUnit enclosure) {
  if (enclosure < 0) {
    throw std::invalid_argument("Enclosure must be non-negative");
  }

  return outer.left() <= inner.left() - enclosure && outer.bottom() <= inner.bottom() - enclosure &&
         outer.right() >= inner.right() + enclosure && outer.top() >= inner.top() + enclosure;
}

}  // namespace aurora::geom
