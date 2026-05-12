#include "drc_lvs/ParasiticExtractor.h"

#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "tech/TechDatabase.h"

#include <map>

namespace aurora::drc_lvs {

ParasiticExtractor::ParasiticExtractor(const tech::TechDatabase& tech) : tech_(tech) {}

ParasiticResult ParasiticExtractor::extract(const db::DbView& view,
                                             const db::DbCellLib& lib) const {
  ParasiticResult result;

  // Collect shapes by layer with bounding boxes
  std::map<db::DbId, std::vector<geom::GeomBox>> layerShapes;
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& box = static_cast<const db::DbRect*>(s)->box();
    if (!box.empty()) layerShapes[s->layerId()].push_back(box);
  }

  // Coupling capacitance: compute overlap area between different layers
  double couplingCapPerArea = 0.0005; // fF/µm² (typical for metal1-metal2)
  const double nm2toUm2 = 1.0 / 1e6;

  for (auto it1 = layerShapes.begin(); it1 != layerShapes.end(); ++it1) {
    for (auto it2 = std::next(it1); it2 != layerShapes.end(); ++it2) {
      const auto* lay1 = lib.findLayer(it1->first);
      const auto* lay2 = lib.findLayer(it2->first);
      if (!lay1 || !lay2) continue;

      for (const auto& b1 : it1->second) {
        for (const auto& b2 : it2->second) {
          if (auto inter = b1.intersection(b2)) {
            double overlapArea = static_cast<double>(inter->width()) * inter->height() * nm2toUm2;
            if (overlapArea > 0) {
              result.caps.push_back({lay1->name(), lay2->name(),
                                     overlapArea * couplingCapPerArea,
                                     static_cast<double>(inter->width() * inter->height())});
            }
          }
        }
      }
    }
  }

  // Wire resistance: R = ρ * L / W
  double sheetResistance = 0.08; // Ω/sq for metal
  for (const auto& [lid, boxes] : layerShapes) {
    const auto* layer = lib.findLayer(lid);
    if (!layer) continue;
    for (const auto& box : boxes) {
      double length = static_cast<double>(std::max(box.width(), box.height()));
      double width = static_cast<double>(std::min(box.width(), box.height()));
      if (width > 0 && length > 0) {
        double sq = length / width;
        result.resistors.push_back({layer->name(), sq * sheetResistance, length});
      }
    }
  }

  result.extracted = true;
  return result;
}

}  // namespace aurora::drc_lvs
