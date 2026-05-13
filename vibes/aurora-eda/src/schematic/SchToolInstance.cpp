#include "schematic/SchToolInstance.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"

namespace aurora::schematic {

SchToolInstance::SchToolInstance(db::DbId masterCellId)
    : SchTool("Place Instance"), masterCellId_(masterCellId) {}

void SchToolInstance::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  if (masterCellId_ == db::kInvalidId) return;
  const auto snapped = ctrl.snap(p);
  auto& view = ctrl.document().view();
  const auto name = ctrl.nextInstanceName();
  db::DbTransform xform;
  xform.dx = snapped.x;
  xform.dy = snapped.y;
  (void)view.createInstance(name, masterCellId_, xform);
}

void SchToolInstance::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = ctrl.snap(p);
}

}  // namespace aurora::schematic
