#include "pdk/MosPcell.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbView.h"
#include "db/DbTypes.h"
#include "geom/GeomBox.h"
#include "tech/TechDatabase.h"

namespace aurora::pdk {

static void generateNmos(db::DbView& lv, const db::DbCellLib& lib,
                          const tech::TechDatabase&, const ParamMap& params) {
  // Parse parameters with defaults
  auto getP = [&](const std::string& key, long long def) -> long long {
    auto it = params.find(key);
    return it != params.end() ? std::stoll(it->second) : def;
  };
  const long long W       = getP("W", 2000);      // total width (nm)
  const long long L       = getP("L", 180);        // gate length (nm)
  const long long fingers = getP("fingers", 1);    // gate fingers
  const long long Wf      = W / fingers;           // width per finger

  // Technology constants
  const long long contSize   = 220;  // contact size
  const long long contEnc    = 150;  // diffusion enclosure of contact
  const long long contToPoly = 180;  // contact-to-poly spacing
  const long long polyExt    = 500;  // poly extension beyond diffusion
  const long long diffH      = L + 2 * (contEnc + contSize + contToPoly);

  // Find layers
  auto findLayer = [&](std::string_view name) -> db::DbId {
    for (const auto lid : lib.layerIds()) {
      const auto* l = lib.findLayer(lid);
      if (l && l->name() == name) return lid;
    }
    return db::kInvalidId;
  };

  const auto diffId = findLayer("diff");
  const auto polyId = findLayer("poly");
  const auto contId = findLayer("contact");

  if (diffId == db::kInvalidId || polyId == db::kInvalidId) return;

  for (long long f = 0; f < fingers; ++f) {
    const long long fx = f * (Wf + 200); // finger pitch

    // Diffusion for this finger
    (void)lv.createRect(diffId, geom::GeomBox{fx, 0, fx + Wf, diffH});

    // Poly gate
    const long long gx = fx + Wf / 2 - L / 2;
    (void)lv.createRect(polyId, geom::GeomBox{gx, -polyExt, gx + L, diffH + polyExt});

    // Source contacts (left side of gate)
    const long long sxStart = fx + contEnc;
    const long long syStart = contEnc;
    const long long sxEnd   = fx + Wf / 2 - contToPoly - contSize;
    const long long numCont = (sxEnd - sxStart + contSize) / (contSize + 300);
    if (numCont > 0) {
      const long long step = (sxEnd - sxStart) / numCont;
      for (long long c = 0; c < numCont; ++c) {
        const long long cx = sxStart + c * step;
        (void)lv.createRect(contId != db::kInvalidId ? contId : diffId,
                            geom::GeomBox{cx, syStart, cx + contSize, syStart + contSize});
        // Top row
        (void)lv.createRect(contId != db::kInvalidId ? contId : diffId,
                            geom::GeomBox{cx, diffH - syStart - contSize,
                                          cx + contSize, diffH - syStart});
      }
    }

    // Drain contacts (right side of gate)
    const long long dxStart = fx + Wf / 2 + contToPoly;
    const long long dxEnd   = fx + Wf - contEnc - contSize;
    const long long numContD = (dxEnd - dxStart + contSize) / (contSize + 300);
    if (numContD > 0) {
      const long long stepD = (dxEnd - dxStart) / numContD;
      for (long long c = 0; c < numContD; ++c) {
        const long long cx = dxStart + c * stepD;
        (void)lv.createRect(contId != db::kInvalidId ? contId : diffId,
                            geom::GeomBox{cx, syStart, cx + contSize, syStart + contSize});
        (void)lv.createRect(contId != db::kInvalidId ? contId : diffId,
                            geom::GeomBox{cx, diffH - syStart - contSize,
                                          cx + contSize, diffH - syStart});
      }
    }
  }
}

void registerMosPcells(PcellRegistry& registry) {
  registry.registerPcell({"NMOS",
      {{"W", "2000"}, {"L", "180"}, {"fingers", "1"}},
      generateNmos});
}

}  // namespace aurora::pdk
