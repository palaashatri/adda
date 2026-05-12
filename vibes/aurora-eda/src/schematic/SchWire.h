#pragma once

#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

#include <vector>

namespace aurora::schematic {

class SchWire {
 public:
  SchWire() = default;
  SchWire(db::DbId netId, std::vector<geom::GeomPoint> points, bool isBus = false);

  [[nodiscard]] db::DbId netId() const;
  [[nodiscard]] const std::vector<geom::GeomPoint>& points() const;
  [[nodiscard]] bool isBus() const { return isBus_; }
  void setBus(bool b) { isBus_ = b; }

 private:
  db::DbId netId_{db::kInvalidId};
  std::vector<geom::GeomPoint> points_;
  bool isBus_{false};
};

}  // namespace aurora::schematic
