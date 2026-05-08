#include "schematic/SchEditorController.h"
#include "schematic/SchTool.h"

#include <format>
#include <stdexcept>

namespace aurora::schematic {

SchEditorController::SchEditorController(SchDocument& document) : document_(&document) {}

SchEditorController::~SchEditorController() = default;

SchDocument& SchEditorController::document() { return *document_; }
const SchDocument& SchEditorController::document() const { return *document_; }

geom::DbUnit SchEditorController::grid() const { return grid_; }

void SchEditorController::setGrid(geom::DbUnit grid) {
  if (grid <= 0) throw std::invalid_argument("Schematic grid must be positive");
  grid_ = grid;
}

void SchEditorController::setActiveTool(std::unique_ptr<SchTool> tool) {
  if (activeTool_) activeTool_->deactivate(*this);
  activeTool_ = std::move(tool);
  if (activeTool_) activeTool_->activate(*this);
}

void SchEditorController::mousePress(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mousePress(*this, p);
}

void SchEditorController::mouseMove(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mouseMove(*this, p);
}

void SchEditorController::mouseRelease(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mouseRelease(*this, p);
}

void SchEditorController::keyPress(SchKeyEvent key) {
  if (activeTool_) activeTool_->keyPress(*this, key);
}

geom::GeomPoint SchEditorController::snap(geom::GeomPoint p) const {
  const auto s = static_cast<long long>(grid_);
  auto round = [s](long long v) { return ((v + s / 2) / s) * s; };
  return {round(p.x), round(p.y)};
}

std::string SchEditorController::nextNetName() {
  return std::format("net_{:04d}", ++netCounter_);
}

std::string SchEditorController::nextInstanceName() {
  return std::format("X{}", instanceCounter_++);
}

}  // namespace aurora::schematic
