#include "layout/LayEditorController.h"

#include <stdexcept>

namespace aurora::layout {

LayEditorController::LayEditorController(LayDocument& document) : document_(&document) {}

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

}  // namespace aurora::layout
