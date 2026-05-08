#pragma once

#include "db/DbView.h"
#include "schematic/SchWire.h"

#include <vector>

namespace aurora::schematic {

class SchDocument {
 public:
  explicit SchDocument(db::DbView& view);

  [[nodiscard]] db::DbView& view();
  [[nodiscard]] const db::DbView& view() const;

  [[nodiscard]] SchWire& addWire(db::DbId netId, std::vector<geom::GeomPoint> points);
  [[nodiscard]] const std::vector<SchWire>& wires() const;

 private:
  db::DbView* view_{nullptr};
  std::vector<SchWire> wires_;
};

}  // namespace aurora::schematic
