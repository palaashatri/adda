#include "layout/LayEditorController.h"
#include "layout/LayTool.h"

#include <stdexcept>

namespace aurora::layout {

LayEditorController::LayEditorController(LayDocument& document) : document_(&document) {}

LayEditorController::~LayEditorController() = default;

LayDocument& LayEditorController::document() {
  return *document_;
}

const LayDocument& LayEditorController::document() const {
  return *document_;
}

geom::DbUnit LayEditorController::grid() const {
  return grid_;
}

double LayEditorController::zoom() const {
  return zoom_;
}

void LayEditorController::setGrid(geom::DbUnit grid) {
  if (grid <= 0) {
    throw std::invalid_argument("Layout grid must be positive");
  }
  grid_ = grid;
}

void LayEditorController::setZoom(double zoom) {
  if (zoom <= 0.0) {
    throw std::invalid_argument("Layout zoom must be positive");
  }
  zoom_ = zoom;
}

void LayEditorController::setActiveLayerId(db::DbId layerId) {
  activeLayerId_ = layerId;
}

void LayEditorController::setActiveTool(std::unique_ptr<LayTool> tool) {
  activeTool_ = std::move(tool);
}

LayTool* LayEditorController::activeTool() const {
  return activeTool_.get();
}

db::DbId LayEditorController::activeLayerId() const {
  return activeLayerId_;
}

void LayEditorController::mousePress(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mousePress(*this, p);
}

void LayEditorController::mouseMove(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mouseMove(*this, p);
}

void LayEditorController::mouseRelease(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mouseRelease(*this, p);
}

}  // namespace aurora::layout
