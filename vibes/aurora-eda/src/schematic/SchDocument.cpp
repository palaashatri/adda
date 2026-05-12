#include "schematic/SchDocument.h"

#include <stdexcept>
#include <utility>

namespace aurora::schematic {

SchDocument::SchDocument(db::DbView& view) : view_(&view) {
  if (view.type() != db::DbViewType::Schematic) {
    throw std::invalid_argument("SchDocument requires a schematic DbView");
  }
}

db::DbView& SchDocument::view() { return *view_; }
const db::DbView& SchDocument::view() const { return *view_; }

SchWire& SchDocument::addWire(db::DbId netId, std::vector<geom::GeomPoint> points) {
  wires_.emplace_back(netId, std::move(points));
  return wires_.back();
}

void SchDocument::removeWireAt(std::size_t index) {
  if (index < wires_.size()) wires_.erase(wires_.begin() + static_cast<std::ptrdiff_t>(index));
}

void SchDocument::clearWires() { wires_.clear(); }

const std::vector<SchWire>& SchDocument::wires() const { return wires_; }
std::vector<SchWire>& SchDocument::wires() { return wires_; }

SchNetLabel& SchDocument::addNetLabel(db::DbId netId, geom::GeomPoint position) {
  netLabels_.push_back({netId, position});
  return netLabels_.back();
}

void SchDocument::removeNetLabelAt(std::size_t index) {
  if (index < netLabels_.size())
    netLabels_.erase(netLabels_.begin() + static_cast<std::ptrdiff_t>(index));
}

void SchDocument::clearNetLabels() { netLabels_.clear(); }

const std::vector<SchNetLabel>& SchDocument::netLabels() const { return netLabels_; }
std::vector<SchNetLabel>& SchDocument::netLabels() { return netLabels_; }

SchStimulus& SchDocument::addStimulus(std::string type, db::DbId netId,
                                       geom::GeomPoint position) {
  stimuli_.push_back({std::move(type), netId, position, {}});
  return stimuli_.back();
}

void SchDocument::removeStimulusAt(std::size_t index) {
  if (index < stimuli_.size())
    stimuli_.erase(stimuli_.begin() + static_cast<std::ptrdiff_t>(index));
}

void SchDocument::clearStimuli() { stimuli_.clear(); }

const std::vector<SchStimulus>& SchDocument::stimuli() const { return stimuli_; }
std::vector<SchStimulus>& SchDocument::stimuli() { return stimuli_; }

}  // namespace aurora::schematic
