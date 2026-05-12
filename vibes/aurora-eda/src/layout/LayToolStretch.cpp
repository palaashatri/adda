#include "layout/LayToolStretch.h"
#include "layout/LayEditorController.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <cstdlib>

namespace aurora::layout {

LayToolStretch::LayToolStretch() : LayTool("Stretch") {}

void LayToolStretch::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  auto& view = ctrl.document().view();
  const geom::DbUnit hitRadius = ctrl.grid() * 3;

  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& box = static_cast<const db::DbRect*>(s)->box();

    // Check if near an edge
    stretchLeft_ = std::abs(p.x - box.left()) < hitRadius && p.y >= box.bottom() && p.y <= box.top();
    stretchRight_ = std::abs(p.x - box.right()) < hitRadius && p.y >= box.bottom() && p.y <= box.top();
    stretchBottom_ = std::abs(p.y - box.bottom()) < hitRadius && p.x >= box.left() && p.x <= box.right();
    stretchTop_ = std::abs(p.y - box.top()) < hitRadius && p.x >= box.left() && p.x <= box.right();

    if (stretchLeft_ || stretchRight_ || stretchBottom_ || stretchTop_) {
      targetShapeId_ = sid;
      stretching_ = true;
      dragStart_ = p;
      cursor_ = p;
      break;
    }
  }
}

void LayToolStretch::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
  if (!stretching_) return;

  auto* s = ctrl.document().view().findShape(targetShapeId_);
  if (!s || s->kind() != db::DbShapeKind::Rect) return;
  auto& r = static_cast<db::DbRect&>(*s);
  auto box = r.box();
  const auto dx = p.x - dragStart_.x;
  const auto dy = p.y - dragStart_.y;
  if (stretchLeft_) box = geom::GeomBox{box.left() + dx, box.bottom(), box.right(), box.top()};
  else if (stretchRight_) box = geom::GeomBox{box.left(), box.bottom(), box.right() + dx, box.top()};
  else if (stretchBottom_) box = geom::GeomBox{box.left(), box.bottom() + dy, box.right(), box.top()};
  else if (stretchTop_) box = geom::GeomBox{box.left(), box.bottom(), box.right(), box.top() + dy};
  // Don't allow flipping
  if (box.width() > 0 && box.height() > 0) r.setBox(box);
  dragStart_ = p;
}

void LayToolStretch::mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) {
  stretching_ = false;
  targetShapeId_ = db::kInvalidId;
}

void LayToolStretch::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) stretching_ = false;
}

}  // namespace aurora::layout
