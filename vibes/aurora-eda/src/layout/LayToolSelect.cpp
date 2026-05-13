#include "layout/LayToolSelect.h"
#include "layout/LayEditorController.h"
#include "layout/LayDocument.h"
#include "db/DbInstance.h"
#include "db/DbView.h"
#include "db/DbShape.h"
#include "geom/GeomBox.h"

#include <algorithm>
#include <cstdlib>

namespace aurora::layout {

LayToolSelect::LayToolSelect() : LayTool("Select") {}

void LayToolSelect::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  pressPoint_ = p;
  rbStart_ = p;
  rbEnd_ = p;
  rubberBand_ = true;
}

void LayToolSelect::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  if (rubberBand_) rbEnd_ = p;
}

void LayToolSelect::mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) {
  rubberBand_ = false;
  if (!pressPoint_) return;

  const auto dx = std::abs(p.x - pressPoint_->x);
  const auto dy = std::abs(p.y - pressPoint_->y);
  const bool isClick = (dx < 200 && dy < 200);

  auto& view = ctrl.document().view();
  selectedShapes_.clear();
  selectedInstances_.clear();

  if (isClick) {
    // Try instances first (they render on top)
    for (const auto instId : view.instanceIds()) {
      const auto* inst = view.findInstance(instId);
      if (!inst) continue;
      const auto ox = inst->transform().dx;
      const auto oy = inst->transform().dy;
      geom::GeomBox instBox{ox - 100, oy - 100, ox + 100, oy + 100};
      if (instBox.contains({p.x, p.y})) {
        selectedInstances_.insert(instId);
        pressPoint_.reset();
        return;
      }
    }
    // Try shapes
    for (const auto shapeId : view.shapeIds()) {
      const auto* shape = view.findShape(shapeId);
      if (!shape) continue;
      geom::GeomBox bb;
      switch (shape->kind()) {
        case db::DbShapeKind::Rect:
          bb = static_cast<const db::DbRect*>(shape)->box();
          break;
        case db::DbShapeKind::Polygon:
          bb = static_cast<const db::DbPolygon*>(shape)->polygon().boundingBox();
          break;
        case db::DbShapeKind::Path: {
          const auto& pts = static_cast<const db::DbPath*>(shape)->path().points();
          if (!pts.empty()) {
            bb = geom::GeomBox{pts.front().x, pts.front().y, pts.front().x, pts.front().y};
            for (const auto& pt : pts)
              bb = geom::GeomBox{std::min(bb.left(), pt.x), std::min(bb.bottom(), pt.y),
                                 std::max(bb.right(), pt.x), std::max(bb.top(), pt.y)};
          }
          break;
        }
        case db::DbShapeKind::Text:
          bb = geom::GeomBox{p.x - 100, p.y - 100, p.x + 100, p.y + 100};
          break;
      }
      if (bb.contains({p.x, p.y})) {
        selectedShapes_.insert(shapeId);
        break;
      }
    }
  } else {
    const geom::DbUnit x0 = std::min(pressPoint_->x, p.x);
    const geom::DbUnit x1 = std::max(pressPoint_->x, p.x);
    const geom::DbUnit y0 = std::min(pressPoint_->y, p.y);
    const geom::DbUnit y1 = std::max(pressPoint_->y, p.y);
    const geom::GeomBox selBox{x0, y0, x1, y1};
    for (const auto shapeId : view.shapeIds()) {
      const auto* shape = view.findShape(shapeId);
      if (!shape) continue;
      if (shape->kind() == db::DbShapeKind::Rect) {
        if (selBox.intersects(static_cast<const db::DbRect*>(shape)->box()))
          selectedShapes_.insert(shapeId);
      }
    }
    for (const auto instId : view.instanceIds()) {
      const auto* inst = view.findInstance(instId);
      if (!inst) continue;
      const auto ox = inst->transform().dx;
      const auto oy = inst->transform().dy;
      geom::GeomBox ib{ox, oy, ox, oy};
      if (selBox.intersects(ib)) selectedInstances_.insert(instId);
    }
  }
  pressPoint_.reset();
}

void LayToolSelect::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) { // Escape
    clearSelection();
    rubberBand_ = false;
    pressPoint_.reset();
    return;
  }
  if ((qtKey == 16777223 || qtKey == 16777219) && (!selectedShapes_.empty() || !selectedInstances_.empty())) {
    auto& view = ctrl.document().view();
    for (const auto id : selectedShapes_) view.removeShape(id);
    for (const auto id : selectedInstances_) view.removeInstance(id);
    clearSelection();
  }
}

}  // namespace aurora::layout
