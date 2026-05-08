#pragma once

#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

#include <vector>

namespace aurora::schematic {

class SchWire {
 public:
  SchWire() = default;
  SchWire(db::DbId netId, std::vector<geom::GeomPoint> points);

  [[nodiscard]] db::DbId netId() const;
  [[nodiscard]] const std::vector<geom::GeomPoint>& points() const;

 private:
  db::DbId netId_{db::kInvalidId};
  std::vector<geom::GeomPoint> points_;
};

}  // namespace aurora::schematic
