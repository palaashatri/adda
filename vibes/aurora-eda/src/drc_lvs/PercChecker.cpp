#include "drc_lvs/PercChecker.h"

#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbNet.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "tech/TechDatabase.h"

#include <algorithm>
#include <map>

namespace aurora::drc_lvs {

PercChecker::PercChecker(const tech::TechDatabase& tech, PercOptions opts)
    : tech_(tech), opts_(std::move(opts)) {}

PercResult PercChecker::run(const db::DbView& view, const db::DbCellLib& lib) const {
  PercResult result;

  // Find power/ground nets.
  bool hasVdd = false, hasVss = false;
  for (auto nid : view.netIds()) {
    const auto* n = view.findNet(nid);
    if (!n) continue;
    if (n->name() == opts_.powerNet) hasVdd = true;
    if (n->name() == opts_.groundNet) hasVss = true;
  }
  if (!hasVdd) {
    result.violations.push_back({DrcViolationType::ERC, "",
        "No power net '" + opts_.powerNet + "' present", {}});
  }
  if (!hasVss) {
    result.violations.push_back({DrcViolationType::ERC, "",
        "No ground net '" + opts_.groundNet + "' present", {}});
  }

  // Sum metal width on each layer; treat narrowest shape as worst-case current path.
  std::map<db::DbId, long long> minWidthPerLayer;
  std::map<db::DbId, long long> totalLengthPerLayer;
  for (auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& box = static_cast<const db::DbRect*>(s)->box();
    if (box.empty()) continue;
    long long w = std::min(box.width(), box.height());
    long long l = std::max(box.width(), box.height());
    auto& mw = minWidthPerLayer[s->layerId()];
    if (mw == 0 || w < mw) mw = w;
    totalLengthPerLayer[s->layerId()] += l;
  }

  // IR drop estimate: V = I * R; R = ρ * L / W. Use thinnest power-layer estimate.
  for (const auto& [lid, minW] : minWidthPerLayer) {
    const auto* layer = lib.findLayer(lid);
    if (!layer) continue;
    if (minW <= 0) continue;
    long long L = totalLengthPerLayer[lid];
    double sq = static_cast<double>(L) / static_cast<double>(minW);
    double R = sq * opts_.sheetResistanceOhm;
    double drop = opts_.estimatedCurrentMa * 1e-3 * R;
    if (drop > result.estimatedIRDrop) result.estimatedIRDrop = drop;
    if (drop > opts_.maxIRDropVolts) {
      result.violations.push_back({DrcViolationType::ERC, layer->name(),
          "IR drop " + std::to_string(drop) + " V exceeds budget", {}});
    }

    // Current density: I / W (mA per µm)
    double widthUm = static_cast<double>(minW) / 1000.0;
    if (widthUm > 0) {
      double j = opts_.estimatedCurrentMa / widthUm;
      if (j > result.worstCurrentDensity) result.worstCurrentDensity = j;
      if (j > opts_.maxCurrentDensity) {
        result.violations.push_back({DrcViolationType::ERC, layer->name(),
            "Current density " + std::to_string(j) + " mA/µm exceeds EM limit", {}});
      }
    }
  }
  return result;
}

}  // namespace aurora::drc_lvs
