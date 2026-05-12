#include "layout/LayEditorController.h"
#include "layout/LayTool.h"

#include <stdexcept>

namespace aurora::layout {

LayEditorController::LayEditorController(LayDocument& document) : document_(&document) {}

LayEditorController::~LayEditorController() = default;

LayDocument& LayEditorController::document() { return *document_; }
const LayDocument& LayEditorController::document() const { return *document_; }

geom::DbUnit LayEditorController::grid() const { return grid_; }
double LayEditorController::zoom() const { return zoom_; }

void LayEditorController::setGrid(geom::DbUnit grid) {
  if (grid <= 0) throw std::invalid_argument("Layout grid must be positive");
  grid_ = grid;
}

void LayEditorController::setZoom(double zoom) {
  if (zoom <= 0.0) throw std::invalid_argument("Layout zoom must be positive");
  zoom_ = zoom;
}

void LayEditorController::setActiveLayerId(db::DbId layerId) { activeLayerId_ = layerId; }

void LayEditorController::setActiveTool(std::unique_ptr<LayTool> tool) {
  activeTool_ = std::move(tool);
}

LayTool* LayEditorController::activeTool() const { return activeTool_.get(); }
db::DbId LayEditorController::activeLayerId() const { return activeLayerId_; }

void LayEditorController::mousePress(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mousePress(*this, snap(p));
}

void LayEditorController::mouseMove(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mouseMove(*this, snap(p));
}

void LayEditorController::mouseRelease(geom::GeomPoint p) {
  if (activeTool_) activeTool_->mouseRelease(*this, snap(p));
}

void LayEditorController::keyPress(int qtKey) {
  if (activeTool_) activeTool_->keyPress(*this, qtKey);
}

geom::GeomPoint LayEditorController::snap(geom::GeomPoint p) const {
  if (snapMode_ == SnapToGrid) {
    const auto s = static_cast<long long>(grid_);
    auto round = [s](long long v) { return ((v + s / 2) / s) * s; };
    return {round(p.x), round(p.y)};
  }
  // Snap to object: find nearest shape edge/center within grid
  geom::DbUnit bestDist = grid_ * 5;
  geom::GeomPoint result = p;
  for (const auto sid : document().view().shapeIds()) {
    const auto* s = document().view().findShape(sid);
    if (!s) continue;
    geom::GeomBox bb;
    if (s->kind() == db::DbShapeKind::Rect)
      bb = static_cast<const db::DbRect*>(s)->box();
    else continue;
    // Check edges and center
    geom::GeomPoint candidates[] = {
      {bb.left(), p.y}, {bb.right(), p.y},
      {p.x, bb.bottom()}, {p.x, bb.top()},
      {(bb.left() + bb.right()) / 2, (bb.bottom() + bb.top()) / 2}
    };
    for (const auto& c : candidates) {
      auto dx = std::abs(c.x - p.x);
      auto dy = std::abs(c.y - p.y);
      auto d = dx + dy;
      if (d < bestDist) { bestDist = d; result = c; }
    }
  }
  return result;
}

}  // namespace aurora::layout
