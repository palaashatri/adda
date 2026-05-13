#include "pdk/PcellLibrary.h"

#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "tech/TechDatabase.h"

namespace aurora::pdk {

namespace {

db::DbId layerOf(const db::DbCellLib& lib, std::string_view name) {
  for (auto lid : lib.layerIds()) {
    const auto* l = lib.findLayer(lid);
    if (l && l->name() == name) return lid;
  }
  return db::kInvalidId;
}

long long getI(const ParamMap& p, const std::string& k, long long def) {
  auto it = p.find(k);
  return it != p.end() ? std::stoll(it->second) : def;
}

void generatePmos(db::DbView& lv, const db::DbCellLib& lib,
                  const tech::TechDatabase&, const ParamMap& p) {
  const long long W = getI(p, "W", 4000);
  const long long L = getI(p, "L", 180);
  const long long nwellEnc = 400;
  const auto diff = layerOf(lib, "diff");
  const auto poly = layerOf(lib, "poly");
  const auto nwell = layerOf(lib, "nwell");
  if (diff == db::kInvalidId || poly == db::kInvalidId) return;
  const long long diffH = L + 1200;
  (void)lv.createRect(diff, geom::GeomBox{0, 0, W, diffH});
  (void)lv.createRect(poly, geom::GeomBox{W/2 - L/2, -500, W/2 + L/2, diffH + 500});
  if (nwell != db::kInvalidId) {
    (void)lv.createRect(nwell, geom::GeomBox{-nwellEnc, -nwellEnc, W + nwellEnc, diffH + nwellEnc});
  }
}

void generatePolyResistor(db::DbView& lv, const db::DbCellLib& lib,
                          const tech::TechDatabase&, const ParamMap& p) {
  const long long W = getI(p, "W", 500);
  const long long L = getI(p, "L", 5000);
  const auto poly = layerOf(lib, "poly");
  const auto cont = layerOf(lib, "contact");
  if (poly == db::kInvalidId) return;
  (void)lv.createRect(poly, geom::GeomBox{0, 0, W, L});
  if (cont != db::kInvalidId) {
    (void)lv.createRect(cont, geom::GeomBox{W/4, 50, W/4 + 220, 270});
    (void)lv.createRect(cont, geom::GeomBox{W/4, L - 270, W/4 + 220, L - 50});
  }
}

void generateMimCap(db::DbView& lv, const db::DbCellLib& lib,
                    const tech::TechDatabase&, const ParamMap& p) {
  const long long W = getI(p, "W", 2000);
  const long long H = getI(p, "H", 2000);
  const auto m1 = layerOf(lib, "metal1");
  const auto m2 = layerOf(lib, "metal2");
  if (m1 == db::kInvalidId || m2 == db::kInvalidId) return;
  (void)lv.createRect(m1, geom::GeomBox{0, 0, W, H});
  (void)lv.createRect(m2, geom::GeomBox{100, 100, W - 100, H - 100});
}

void generateInductor(db::DbView& lv, const db::DbCellLib& lib,
                      const tech::TechDatabase&, const ParamMap& p) {
  const long long turns = getI(p, "turns", 3);
  const long long pitch = getI(p, "pitch", 600);
  const long long width = getI(p, "width", 200);
  const auto m = layerOf(lib, "metal2");
  if (m == db::kInvalidId) return;
  long long off = 0;
  for (long long t = 0; t < turns; ++t) {
    const long long s = off, e = (turns - t) * pitch * 2 - off;
    (void)lv.createRect(m, geom::GeomBox{s, s, e, s + width});
    (void)lv.createRect(m, geom::GeomBox{e - width, s, e, e});
    (void)lv.createRect(m, geom::GeomBox{s, e - width, e, e});
    (void)lv.createRect(m, geom::GeomBox{s, s, s + width, e});
    off += pitch;
  }
}

void generateNpnBjt(db::DbView& lv, const db::DbCellLib& lib,
                     const tech::TechDatabase&, const ParamMap& p) {
  const long long size = getI(p, "size", 3000);
  const auto diff = layerOf(lib, "diff");
  const auto nwell = layerOf(lib, "nwell");
  if (diff == db::kInvalidId) return;
  // Emitter (small center), base (mid ring), collector (outer ring).
  (void)lv.createRect(diff, geom::GeomBox{size/3, size/3, 2*size/3, 2*size/3});  // emitter
  if (nwell != db::kInvalidId) {
    (void)lv.createRect(nwell, geom::GeomBox{0, 0, size, size});                 // collector well
  }
}

void generatePnDiode(db::DbView& lv, const db::DbCellLib& lib,
                      const tech::TechDatabase&, const ParamMap& p) {
  const long long W = getI(p, "W", 1000);
  const long long H = getI(p, "H", 1000);
  const auto diff = layerOf(lib, "diff");
  const auto nwell = layerOf(lib, "nwell");
  if (diff == db::kInvalidId) return;
  (void)lv.createRect(diff, geom::GeomBox{0, 0, W, H});
  if (nwell != db::kInvalidId) {
    (void)lv.createRect(nwell, geom::GeomBox{-200, -200, W + 200, H + 200});
  }
}

void generateCommonCentroid(db::DbView& lv, const db::DbCellLib& lib,
                             const tech::TechDatabase& tech, const ParamMap& p) {
  // 2x2 ABBA pattern of NMOS unit cells.
  const long long W = getI(p, "Wunit", 2000);
  const long long L = getI(p, "L", 180);
  const long long pitch = W + 1000;
  const auto diff = layerOf(lib, "diff");
  const auto poly = layerOf(lib, "poly");
  if (diff == db::kInvalidId || poly == db::kInvalidId) return;
  // ABBA pattern — minimum-viable: just draw 4 unit cells in 2x2 grid.
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      long long ox = i * pitch;
      long long oy = j * pitch;
      (void)lv.createRect(diff, geom::GeomBox{ox, oy, ox + W, oy + 1200});
      (void)lv.createRect(poly, geom::GeomBox{ox + W/2 - L/2, oy - 500,
                                              ox + W/2 + L/2, oy + 1700});
    }
  }
  (void)tech;
}

}  // namespace

void registerPcellLibrary(PcellRegistry& registry) {
  registry.registerPcell({"PMOS",
      {{"W", "4000"}, {"L", "180"}}, generatePmos});
  registry.registerPcell({"RES_POLY",
      {{"W", "500"}, {"L", "5000"}}, generatePolyResistor});
  registry.registerPcell({"CAP_MIM",
      {{"W", "2000"}, {"H", "2000"}}, generateMimCap});
  registry.registerPcell({"IND_SPIRAL",
      {{"turns", "3"}, {"pitch", "600"}, {"width", "200"}}, generateInductor});
  registry.registerPcell({"BJT_NPN",
      {{"size", "3000"}}, generateNpnBjt});
  registry.registerPcell({"DIODE_PN",
      {{"W", "1000"}, {"H", "1000"}}, generatePnDiode});
  registry.registerPcell({"MATCH_CC",
      {{"Wunit", "2000"}, {"L", "180"}}, generateCommonCentroid});
}

std::vector<StretchHandle> defaultStretchHandlesFor(std::string_view name) {
  std::vector<StretchHandle> handles;
  if (name == "NMOS" || name == "PMOS") {
    handles.push_back({"W", StretchHandle::Axis::X, 0, 0, 1.0});
    handles.push_back({"L", StretchHandle::Axis::Y, 0, 0, 1.0});
  } else if (name == "RES_POLY") {
    handles.push_back({"L", StretchHandle::Axis::Y, 0, 0, 1.0});
    handles.push_back({"W", StretchHandle::Axis::X, 0, 0, 1.0});
  } else if (name == "CAP_MIM") {
    handles.push_back({"W", StretchHandle::Axis::X, 0, 0, 1.0});
    handles.push_back({"H", StretchHandle::Axis::Y, 0, 0, 1.0});
  }
  return handles;
}

}  // namespace aurora::pdk
