#include "layout/LayToolViaArray.h"
#include "layout/LayEditorController.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <algorithm>

namespace aurora::layout {

LayToolViaArray::LayToolViaArray() : LayTool("Via Array") {}

void LayToolViaArray::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  firstPoint_ = p;
  cursor_ = p;
}

void LayToolViaArray::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
}

void LayToolViaArray::mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) {
  if (!firstPoint_) return;

  const geom::DbUnit l = std::min(firstPoint_->x, p.x);
  const geom::DbUnit r = std::max(firstPoint_->x, p.x);
  const geom::DbUnit b = std::min(firstPoint_->y, p.y);
  const geom::DbUnit t = std::max(firstPoint_->y, p.y);

  firstPoint_.reset();
  cursor_ = p;

  if (l == r || b == t) return;

  // Ask for params via callback
  if (!requestParams) return;
  const auto maybeParams = requestParams();
  if (!maybeParams) return;

  const auto& pr = *maybeParams;
  auto& view = ctrl.document().view();
  const auto layerId = ctrl.activeLayerId();

  // Compute starting position (center the array in the rectangle)
  const geom::DbUnit totalW = pr.columns * pr.viaSize + (pr.columns - 1) * pr.spacingX;
  const geom::DbUnit totalH = pr.rows * pr.viaSize + (pr.rows - 1) * pr.spacingY;
  const geom::DbUnit startX = (l + r - totalW) / 2;
  const geom::DbUnit startY = (b + t - totalH) / 2;

  for (int col = 0; col < pr.columns; ++col) {
    for (int row = 0; row < pr.rows; ++row) {
      const geom::DbUnit vx = startX + col * (pr.viaSize + pr.spacingX);
      const geom::DbUnit vy = startY + row * (pr.viaSize + pr.spacingY);
      (void)view.createRect(layerId,
          geom::GeomBox{vx, vy, vx + pr.viaSize, vy + pr.viaSize});
    }
  }
}

void LayToolViaArray::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) { // Escape
    firstPoint_.reset();
  }
}

}  // namespace aurora::layout
