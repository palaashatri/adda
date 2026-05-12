#include "schematic/SchToolProbe.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"
#include "db/DbConstraint.h"

#include <algorithm>
#include <cmath>

namespace aurora::schematic {

SchToolProbe::SchToolProbe() : SchTool("Probe") {}

void SchToolProbe::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  const geom::DbUnit hitRadius = ctrl.grid() * 10;
  geom::GeomPoint hitPoint;
  db::DbId hitNetId = db::kInvalidId;
  geom::DbUnit bestDist = hitRadius;

  for (const auto& wire : ctrl.document().wires()) {
    const auto& pts = wire.points();
    for (std::size_t i = 1; i < pts.size(); ++i) {
      const auto& a = pts[i - 1];
      const auto& b = pts[i];
      const auto abx = b.x - a.x, aby = b.y - a.y;
      if (abx == 0 && aby == 0) continue;
      const double t = std::clamp(static_cast<double>((p.x - a.x) * abx + (p.y - a.y) * aby) /
                                   static_cast<double>(abx * abx + aby * aby), 0.0, 1.0);
      const geom::DbUnit cx = static_cast<geom::DbUnit>(a.x + t * abx);
      const geom::DbUnit cy = static_cast<geom::DbUnit>(a.y + t * aby);
      const auto dist = static_cast<geom::DbUnit>(
          std::sqrt(static_cast<double>((cx - p.x) * (cx - p.x) + (cy - p.y) * (cy - p.y))));
      if (dist < bestDist) { bestDist = dist; hitPoint = {cx, cy}; hitNetId = wire.netId(); }
    }
  }

  if (hitNetId == db::kInvalidId || !requestType) return;
  auto type = requestType();
  if (!type) return;

  auto& stim = ctrl.document().view().createConstraint(*type);
  stim.addObject(hitNetId);
  stim.setProperty("x", std::to_string(hitPoint.x));
  stim.setProperty("y", std::to_string(hitPoint.y));
}

void SchToolProbe::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) { cursor_ = p; }
void SchToolProbe::keyPress(SchEditorController& ctrl, SchKeyEvent key) { (void)ctrl; (void)key; }

}  // namespace aurora::schematic
