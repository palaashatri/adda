#include "layout/LayToolRect.h"
#include "layout/LayEditorController.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include <algorithm>

namespace aurora::layout {

LayToolRect::LayToolRect() : LayTool("Create Rectangle") {}

void LayToolRect::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  firstPoint_ = p;
}

void LayToolRect::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  // Could implement a preview here
}

void LayToolRect::mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) {
  if (firstPoint_) {
    auto& doc = ctrl.document();
    auto& view = doc.view();
    
    geom::DbUnit l = std::min(firstPoint_->x, p.x);
    geom::DbUnit r = std::max(firstPoint_->x, p.x);
    geom::DbUnit b = std::min(firstPoint_->y, p.y);
    geom::DbUnit t = std::max(firstPoint_->y, p.y);
    
    if (l != r && b != t) {
      (void)view.createRect(ctrl.activeLayerId(), geom::GeomBox{l, b, r, t});
    }
    
    firstPoint_.reset();
  }
}

}  // namespace aurora::layout
