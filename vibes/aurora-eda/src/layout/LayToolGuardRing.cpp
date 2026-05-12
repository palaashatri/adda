#include "layout/LayToolGuardRing.h"
#include "layout/LayEditorController.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <algorithm>

namespace aurora::layout {

LayToolGuardRing::LayToolGuardRing() : LayTool("Guard Ring") {}

void LayToolGuardRing::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  firstPoint_ = p;
  cursor_ = p;
}

void LayToolGuardRing::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
}

void LayToolGuardRing::mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) {
  if (!firstPoint_) return;

  const geom::DbUnit l = std::min(firstPoint_->x, p.x);
  const geom::DbUnit r = std::max(firstPoint_->x, p.x);
  const geom::DbUnit b = std::min(firstPoint_->y, p.y);
  const geom::DbUnit t = std::max(firstPoint_->y, p.y);

  firstPoint_.reset();
  cursor_ = p;

  if (l == r || b == t) return;

  if (!requestParams) return;
  const auto maybeP = requestParams();
  if (!maybeP) return;

  const auto& pr = *maybeP;
  auto& view = ctrl.document().view();
  const auto layerId = ctrl.activeLayerId();

  // Guard ring = outer rect - inner rect (subtraction by drawing 4 rects)
  const geom::DbUnit rw = pr.ringWidth;
  const geom::DbUnit sp = pr.spacing;

  // Top bar
  (void)view.createRect(layerId, geom::GeomBox{l - sp - rw, t + sp, r + sp + rw, t + sp + rw});
  // Bottom bar
  (void)view.createRect(layerId, geom::GeomBox{l - sp - rw, b - sp - rw, r + sp + rw, b - sp});
  // Left bar
  (void)view.createRect(layerId, geom::GeomBox{l - sp - rw, b - sp, l - sp, t + sp});
  // Right bar
  (void)view.createRect(layerId, geom::GeomBox{r + sp, b - sp, r + sp + rw, t + sp});
}

void LayToolGuardRing::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) firstPoint_.reset();
}

}  // namespace aurora::layout
