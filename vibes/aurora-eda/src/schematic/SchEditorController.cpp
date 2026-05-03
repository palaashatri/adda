#include "schematic/SchEditorController.h"

#include <stdexcept>

namespace aurora::schematic {

SchEditorController::SchEditorController(SchDocument& document) : document_(&document) {}

SchDocument& SchEditorController::document() {
  return *document_;
}

const SchDocument& SchEditorController::document() const {
  return *document_;
}

geom::DbUnit SchEditorController::grid() const {
  return grid_;
}

void SchEditorController::setGrid(geom::DbUnit grid) {
  if (grid <= 0) {
    throw std::invalid_argument("Schematic grid must be positive");
  }
  grid_ = grid;
}

}  // namespace aurora::schematic
