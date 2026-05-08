#include "schematic/SchToolSelect.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"

#include <algorithm>
#include <cstdlib>

namespace aurora::schematic {

SchToolSelect::SchToolSelect() : SchTool("Select") {}

void SchToolSelect::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  pressPoint_ = p;
  rbStart_ = p;
  rbEnd_ = p;
  rubberBand_ = true;
}

void SchToolSelect::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) {
  if (rubberBand_) rbEnd_ = p;
}

void SchToolSelect::mouseRelease(SchEditorController& ctrl, geom::GeomPoint p) {
  rubberBand_ = false;
  if (!pressPoint_) return;

  const auto dx = std::abs(p.x - pressPoint_->x);
  const auto dy = std::abs(p.y - pressPoint_->y);
  const bool isClick = (dx < ctrl.grid() * 3 && dy < ctrl.grid() * 3);

  auto& view = ctrl.document().view();
  selected_.clear();

  if (isClick) {
    const geom::DbUnit hitRadius = ctrl.grid() * 8;
    for (const auto instId : view.instanceIds()) {
      const auto* inst = view.findInstance(instId);
      if (!inst) continue;
      if (std::abs(inst->transform().dx - p.x) <= hitRadius &&
          std::abs(inst->transform().dy - p.y) <= hitRadius) {
        selected_.insert(instId);
        break;
      }
    }
  } else {
    const geom::DbUnit x0 = std::min(pressPoint_->x, p.x);
    const geom::DbUnit x1 = std::max(pressPoint_->x, p.x);
    const geom::DbUnit y0 = std::min(pressPoint_->y, p.y);
    const geom::DbUnit y1 = std::max(pressPoint_->y, p.y);
    for (const auto instId : view.instanceIds()) {
      const auto* inst = view.findInstance(instId);
      if (!inst) continue;
      const auto tx = inst->transform().dx;
      const auto ty = inst->transform().dy;
      if (tx >= x0 && tx <= x1 && ty >= y0 && ty <= y1)
        selected_.insert(instId);
    }
  }
  pressPoint_.reset();
}

void SchToolSelect::keyPress(SchEditorController& ctrl, SchKeyEvent key) {
  if (key == SchKeyEvent::Escape) {
    selected_.clear();
    rubberBand_ = false;
    pressPoint_.reset();
    return;
  }
  if (key == SchKeyEvent::Delete && !selected_.empty()) {
    auto& view = ctrl.document().view();
    for (const auto id : selected_) view.removeInstance(id);
    selected_.clear();
  }
}

}  // namespace aurora::schematic
