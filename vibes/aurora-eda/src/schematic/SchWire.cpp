#include "schematic/SchWire.h"

#include <utility>

namespace aurora::schematic {

SchWire::SchWire(db::DbId netId, std::vector<geom::GeomPoint> points)
    : netId_(netId), points_(std::move(points)) {}

db::DbId SchWire::netId() const {
  return netId_;
}

const std::vector<geom::GeomPoint>& SchWire::points() const {
  return points_;
}

}  // namespace aurora::schematic
