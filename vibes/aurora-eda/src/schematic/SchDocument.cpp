#include "schematic/SchDocument.h"

#include <stdexcept>
#include <utility>

namespace aurora::schematic {

SchDocument::SchDocument(db::DbView& view) : view_(&view) {
  if (view.type() != db::DbViewType::Schematic) {
    throw std::invalid_argument("SchDocument requires a schematic DbView");
  }
}

db::DbView& SchDocument::view() {
  return *view_;
}

const db::DbView& SchDocument::view() const {
  return *view_;
}

SchWire& SchDocument::addWire(db::DbId netId, std::vector<geom::GeomPoint> points) {
  wires_.emplace_back(netId, std::move(points));
  return wires_.back();
}

const std::vector<SchWire>& SchDocument::wires() const {
  return wires_;
}

}  // namespace aurora::schematic
