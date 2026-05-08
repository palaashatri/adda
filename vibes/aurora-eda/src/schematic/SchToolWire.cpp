#include "schematic/SchToolWire.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"

namespace aurora::schematic {

SchToolWire::SchToolWire() : SchTool("Wire") {}

void SchToolWire::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  const auto snapped = ctrl.snap(p);
  if (!drawing_) {
    startPoint_ = snapped;
    cursor_ = snapped;
    drawing_ = true;
  } else {
    if (startPoint_.x != snapped.x || startPoint_.y != snapped.y) {
      auto& doc = ctrl.document();
      auto& view = doc.view();
      auto& net = view.createNet(ctrl.nextNetName());
      doc.addWire(net.id(), {startPoint_, snapped});
    }
    startPoint_ = snapped;
  }
}

void SchToolWire::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = ctrl.snap(p);
}

void SchToolWire::keyPress(SchEditorController& ctrl, SchKeyEvent key) {
  if (key == SchKeyEvent::Escape || key == SchKeyEvent::Enter) {
    drawing_ = false;
  }
}

void SchToolWire::deactivate(SchEditorController& ctrl) {
  drawing_ = false;
}

}  // namespace aurora::schematic
