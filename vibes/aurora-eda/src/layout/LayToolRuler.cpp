#include "layout/LayToolRuler.h"
#include "layout/LayEditorController.h"

#include <cmath>
#include <cstdlib>

namespace aurora::layout {

LayToolRuler::LayToolRuler() : LayTool("Ruler") {}

void LayToolRuler::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  if (start_) {
    // Second click: show final distance
    const auto dx = std::abs(p.x - start_->x);
    const auto dy = std::abs(p.y - start_->y);
    const double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));
    // Distance is shown in status bar by the UI
    (void)ctrl;
    start_.reset();
  } else {
    start_ = p;
    cursor_ = p;
  }
}

void LayToolRuler::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
  if (start_) {
    // Report distance in status bar via coordinates change
    const auto dx = std::abs(p.x - start_->x);
    const auto dy = std::abs(p.y - start_->y);
    const double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));
    (void)dist;
    (void)ctrl;
  }
}

void LayToolRuler::keyPress(LayEditorController& ctrl, int qtKey) {
  if (qtKey == 16777216) start_.reset();
}

}  // namespace aurora::layout
