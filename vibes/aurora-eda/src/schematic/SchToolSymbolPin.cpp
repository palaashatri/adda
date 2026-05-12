#include "schematic/SchToolSymbolPin.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbPin.h"
#include "db/DbView.h"

namespace aurora::schematic {

SchToolSymbolPin::SchToolSymbolPin() : SchTool("Symbol Pin") {}

void SchToolSymbolPin::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  if (!requestPin) return;
  auto pinDef = requestPin();
  if (!pinDef) return;

  // Determine direction enum
  auto dir = db::DbPinDirection::Passive;
  if (pinDef->direction == "input") dir = db::DbPinDirection::Input;
  else if (pinDef->direction == "output") dir = db::DbPinDirection::Output;
  else if (pinDef->direction == "inout") dir = db::DbPinDirection::InOut;

  // Create the pin and a visual marker (small rect) at the click position
  auto& view = ctrl.document().view();
  auto& pin = view.createPin(pinDef->name, dir);
  const geom::DbUnit pinSize = 200;
  auto& pinShape = view.createRect(pin.id(), geom::GeomBox{p.x - pinSize / 2, p.y - pinSize / 2,
                                                            p.x + pinSize / 2, p.y + pinSize / 2});
  pin.addShape(pinShape.id());
}

void SchToolSymbolPin::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) { cursor_ = p; }
void SchToolSymbolPin::keyPress(SchEditorController& ctrl, SchKeyEvent key) { (void)ctrl; (void)key; }

}  // namespace aurora::schematic
