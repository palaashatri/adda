#include "schematic/SchToolWire.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"
#include "db/DbNet.h"

#include <format>

namespace aurora::schematic {

SchToolWire::SchToolWire() : SchTool("Wire") {}

void SchToolWire::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  const auto snapped = ctrl.snap(p);
  if (!drawing_) {
    startPoint_ = snapped;
    cursor_ = snapped;
    drawing_ = true;
    // Reset current net — will be created on first segment
    currentNetId_ = db::kInvalidId;
  } else {
    if (startPoint_.x != snapped.x || startPoint_.y != snapped.y) {
      auto& doc = ctrl.document();
      auto& view = doc.view();
      // Use the same net for all segments
      if (currentNetId_ == db::kInvalidId) {
        std::string netName;
        if (busMode_) {
          static int busCounter = 0;
          netName = std::format("BUS_{:02d}<{}:{}>", ++busCounter, busMsb_, busLsb_);
        } else {
          netName = ctrl.nextNetName();
        }
        currentNetId_ = view.createNet(netName).id();
      }
      (void)doc.addWire(currentNetId_, {startPoint_, snapped}, busMode_);
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
    currentNetId_ = db::kInvalidId;
  }
}

void SchToolWire::deactivate(SchEditorController& ctrl) {
  drawing_ = false;
  currentNetId_ = db::kInvalidId;
}

}  // namespace aurora::schematic