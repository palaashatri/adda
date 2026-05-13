#include "layout/LayToolPolygon.h"
#include "layout/LayEditorController.h"
#include "layout/LayDocument.h"
#include "db/DbView.h"
#include "geom/GeomPolygon.h"

namespace aurora::layout {

LayToolPolygon::LayToolPolygon() : LayTool("Polygon") {}

void LayToolPolygon::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  points_.push_back(p);
}

void LayToolPolygon::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
}

void LayToolPolygon::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) { // Escape
    cancel();
    return;
  }
  if ((qtKey == 16777220 || qtKey == 16777221) && points_.size() >= 3) { // Return/Enter
    commitPolygon(ctrl);
  }
}

void LayToolPolygon::commitPolygon(LayEditorController& ctrl) {
  if (points_.size() < 3) { cancel(); return; }
  geom::GeomPolygon poly;
  for (const auto& pt : points_) poly.addPoint(pt);
  (void)ctrl.document().view().createPolygon(ctrl.activeLayerId(), poly);
  cancel();
}

void LayToolPolygon::cancel() {
  points_.clear();
}

}  // namespace aurora::layout
