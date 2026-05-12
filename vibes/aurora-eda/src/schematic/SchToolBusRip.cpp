#include "schematic/SchToolBusRip.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"
#include "db/DbNet.h"

#include <algorithm>
#include <cmath>

namespace aurora::schematic {

SchToolBusRip::SchToolBusRip() : SchTool("Bus Rip") {}

void SchToolBusRip::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  const geom::DbUnit hitRadius = ctrl.grid() * 10;
  geom::GeomPoint hitPoint;
  db::DbId hitNetId = db::kInvalidId;
  bool hitIsBus = false;
  geom::DbUnit bestDist = hitRadius;

  for (const auto& wire : ctrl.document().wires()) {
    if (!wire.isBus()) continue;
    const auto& pts = wire.points();
    for (std::size_t i = 1; i < pts.size(); ++i) {
      const auto& a = pts[i - 1], & b = pts[i];
      const auto abx = b.x - a.x, aby = b.y - a.y;
      if (abx == 0 && aby == 0) continue;
      const double t = std::clamp(static_cast<double>((p.x - a.x) * abx + (p.y - a.y) * aby) /
                                   static_cast<double>(abx * abx + aby * aby), 0.0, 1.0);
      const geom::DbUnit cx = static_cast<geom::DbUnit>(a.x + t * abx);
      const geom::DbUnit cy = static_cast<geom::DbUnit>(a.y + t * aby);
      const auto dist = static_cast<geom::DbUnit>(
          std::sqrt(static_cast<double>((cx - p.x) * (cx - p.x) + (cy - p.y) * (cy - p.y))));
      if (dist < bestDist) { bestDist = dist; hitPoint = {cx, cy}; hitNetId = wire.netId(); hitIsBus = true; }
    }
  }

  if (hitNetId == db::kInvalidId || !hitIsBus || !requestSignalName) return;
  auto sigName = requestSignalName();
  if (!sigName || sigName->empty()) return;

  auto& view = ctrl.document().view();
  auto& ripNet = view.createNet(*sigName);
  // Create a short wire segment from bus to the named signal
  const geom::DbUnit ripLen = ctrl.grid() * 4;
  geom::GeomPoint ripEnd = hitPoint;
  ripEnd.x += ripLen;
  ctrl.document().addWire(ripNet.id(), {hitPoint, ripEnd});
  // Add a net label
  ctrl.document().addNetLabel(ripNet.id(), ripEnd);
}

void SchToolBusRip::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) { cursor_ = p; }
void SchToolBusRip::keyPress(SchEditorController& ctrl, SchKeyEvent key) { (void)ctrl; (void)key; }

}  // namespace aurora::schematic
