#include "schematic/SchToolWire.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"

#include <format>

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
      std::string netName;
      if (busMode_) {
        // Bus mode: name like "BUS_1_00<7:0>"
        static int busCounter = 0;
        netName = std::format("BUS_{:02d}<7:0>", ++busCounter);
      } else {
        netName = ctrl.nextNetName();
      }
      auto& net = view.createNet(netName);
      doc.addWire(net.id(), {startPoint_, snapped}, busMode_);
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
