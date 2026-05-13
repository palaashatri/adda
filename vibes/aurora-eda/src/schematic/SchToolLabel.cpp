#include "schematic/SchToolLabel.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchDocument.h"
#include "db/DbNet.h"
#include "db/DbView.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace aurora::schematic {

SchToolLabel::SchToolLabel() : SchTool("Label") {}

void SchToolLabel::mousePress(SchEditorController& ctrl, geom::GeomPoint p) {
  // Find the nearest wire within hit radius
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
      const double t = static_cast<double>((p.x - a.x) * abx + (p.y - a.y) * aby) /
                       static_cast<double>(abx * abx + aby * aby);
      const double clampedT = std::clamp(t, 0.0, 1.0);
      const geom::DbUnit cx = static_cast<geom::DbUnit>(a.x + clampedT * abx);
      const geom::DbUnit cy = static_cast<geom::DbUnit>(a.y + clampedT * aby);
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

  // Callback to UI to get the label name
  if (requestLabelName) {
    const std::string name = requestLabelName(hitPoint);
    if (!name.empty()) {
      auto* net = ctrl.document().view().findNet(hitNetId);
      if (net) {
        (void)ctrl.document().addNetLabel(hitNetId, hitPoint);
        net->setName(name);
      }
    }
  }
}

void SchToolLabel::mouseMove(SchEditorController& ctrl, geom::GeomPoint p) {
  cursor_ = p;
}

void SchToolLabel::keyPress(SchEditorController& ctrl, SchKeyEvent key) {
  (void)ctrl;
  (void)key;
}

}  // namespace aurora::schematic
