#include "layout/LayToolPath.h"
#include "layout/LayEditorController.h"
#include "layout/LayDocument.h"
#include "db/DbView.h"
#include "geom/GeomPath.h"

namespace aurora::layout {

LayToolPath::LayToolPath() : LayTool("Create Path") {}

void LayToolPath::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  points_.push_back(p);
}

void LayToolPath::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
}

void LayToolPath::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) { // Escape
    cancel();
    return;
  }
  if ((qtKey == 16777220 || qtKey == 16777221) && points_.size() >= 2) { // Return/Enter
    commitPath(ctrl);
  }
}

void LayToolPath::commitPath(LayEditorController& ctrl) {
  if (points_.size() < 2) { cancel(); return; }
  geom::GeomPath path(points_, width_, toGeomCornerStyle(cornerStyle_));
  (void)ctrl.document().view().createPath(ctrl.activeLayerId(), path);
  cancel();
}

void LayToolPath::cancel() {
  points_.clear();
}

}  // namespace aurora::layout
