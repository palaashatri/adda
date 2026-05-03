#include "geom/GeomOps.h"

#include <cassert>
#include <numeric>

namespace {

[[nodiscard]] aurora::geom::DbUnit area(const aurora::geom::GeomBox& box) {
  return box.width() * box.height();
}

}  // namespace

int main() {
  using aurora::geom::GeomBox;
  using aurora::geom::GeomPoint;
  using aurora::geom::GeomPolygon;

  assert(aurora::geom::snapCoordinate(24, 10) == 20);
  assert(aurora::geom::snapCoordinate(26, 10) == 30);
  assert(aurora::geom::snapCoordinate(-26, 10) == -30);
  assert((aurora::geom::snapToGrid(GeomPoint{23, 47}, 10) == GeomPoint{20, 50}));

  const GeomBox a{0, 0, 100, 100};
  const GeomBox b{100, 0, 200, 100};
  const auto merged = aurora::geom::boxUnion(a, b);
  assert(merged.size() == 1);
  assert(merged.front().left() == 0);
  assert(merged.front().right() == 200);

  const GeomBox cutter{25, 25, 75, 75};
  const auto difference = aurora::geom::boxDifference(a, cutter);
  assert(difference.size() == 4);
  const auto remainingArea =
      std::accumulate(difference.begin(), difference.end(), aurora::geom::DbUnit{0},
                      [](auto sum, const auto& box) { return sum + area(box); });
  assert(remainingArea == area(a) - area(cutter));

  const auto intersection = aurora::geom::boxIntersection(a, cutter);
  assert(intersection.has_value());
  assert(intersection->width() == 50);
  assert(intersection->height() == 50);

  assert(aurora::geom::meetsMinimumWidth(a, 100));
  assert(!aurora::geom::meetsMinimumWidth(a, 101));
  assert(aurora::geom::meetsMinimumSpacing(GeomBox{0, 0, 10, 10}, GeomBox{20, 0, 30, 10}, 10));
  assert(!aurora::geom::meetsMinimumSpacing(GeomBox{0, 0, 10, 10}, GeomBox{19, 0, 30, 10}, 10));
  assert(aurora::geom::hasEnclosure(GeomBox{0, 0, 100, 100}, GeomBox{10, 10, 90, 90}, 10));
  assert(!aurora::geom::hasEnclosure(GeomBox{0, 0, 100, 100}, GeomBox{9, 10, 90, 90}, 10));

  assert((aurora::geom::isManhattan(GeomPolygon{{GeomPoint{0, 0}, GeomPoint{10, 0},
                                                 GeomPoint{10, 10}, GeomPoint{0, 10}}})));
  assert(!aurora::geom::isManhattan(
      GeomPolygon{{GeomPoint{0, 0}, GeomPoint{10, 10}, GeomPoint{0, 10}}}));

  return 0;
}
