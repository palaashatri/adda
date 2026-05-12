#include "schematic/SchToolStimulus.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbView.h"
#include "db/DbConstraint.h"

#include <algorithm>
#include <cmath>

namespace aurora::schematic {

SchToolStimulus::SchToolStimulus() : SchTool("Stimulus") {}

void SchToolStimulus::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  const geom::DbUnit hitRadius = ctrl.grid() * 10;
  geom::GeomPoint hitPoint;
  db::DbId hitNetId = db::kInvalidId;
  geom::DbUnit bestDist = hitRadius;

  for (const auto& wire : ctrl.document().wires()) {
    const auto& pts = wire.points();
    for (std::size_t i = 1; i < pts.size(); ++i) {
      const auto& a = pts[i - 1];
      const auto& b = pts[i];
      const auto abx = b.x - a.x;
      const auto aby = b.y - a.y;
      if (abx == 0 && aby == 0) continue;
      const double t = std::clamp(static_cast<double>((p.x - a.x) * abx + (p.y - a.y) * aby) /
                                   static_cast<double>(abx * abx + aby * aby), 0.0, 1.0);
      const geom::DbUnit cx = static_cast<geom::DbUnit>(a.x + t * abx);
      const geom::DbUnit cy = static_cast<geom::DbUnit>(a.y + t * aby);
      const auto dx = std::abs(cx - p.x);
      const auto dy = std::abs(cy - p.y);
      const auto dist = static_cast<geom::DbUnit>(std::sqrt(static_cast<double>(dx * dx + dy * dy)));
      if (dist < bestDist) {
        bestDist = dist;
        hitPoint = {cx, cy};
        hitNetId = wire.netId();
      }
    }
  }

  if (hitNetId == db::kInvalidId) return;

  if (requestParams) {
    auto params = requestParams();
    if (params) {
      auto& stim = ctrl.document().view().createConstraint(params->type);
      stim.addObject(hitNetId);
      stim.setProperty("x", std::to_string(hitPoint.x));
      stim.setProperty("y", std::to_string(hitPoint.y));
      for (const auto& [k, v] : params->values)
        stim.setProperty(k, v);
    }
  }
}

void SchToolStimulus::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
}

void SchToolStimulus::keyPress(SchEditorController& ctrl, SchKeyEvent key) {
  (void)ctrl;
  (void)key;
}

}  // namespace aurora::schematic
