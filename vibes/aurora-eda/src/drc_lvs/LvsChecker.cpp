#include "drc_lvs/LvsChecker.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"

#include <algorithm>
#include <set>
#include <sstream>

namespace aurora::drc_lvs {

LvsResult LvsChecker::compare(const db::DbView& schView, const db::DbView& layView,
                                const db::DbCellLib& lib) const {
  LvsResult result;

  auto collectNets = [](const db::DbView& view) -> std::set<std::string> {
    std::set<std::string> names;
    for (const auto netId : view.netIds()) {
      const auto* net = view.findNet(netId);
      if (net) names.insert(net->name());
    }
    return names;
  };

  const auto schNets = collectNets(schView);
  const auto layNets = collectNets(layView);

  if (schNets.size() != layNets.size()) {
    std::ostringstream msg;
    msg << "Net count mismatch: schematic=" << schNets.size() << ", layout=" << layNets.size();
    result.errors.push_back(msg.str());
  }

  for (const auto& name : schNets)
    if (layNets.find(name) == layNets.end())
      result.errors.push_back("Net in schematic but not layout: " + name);

  for (const auto& name : layNets)
    if (schNets.find(name) == schNets.end())
      result.errors.push_back("Net in layout but not schematic: " + name);

  auto pinCount = [](const db::DbView& view, const std::string& netName) -> std::size_t {
    for (const auto netId : view.netIds()) {
      const auto* net = view.findNet(netId);
      if (net && net->name() == netName) return net->pinIds().size();
    }
    return 0;
  };

  for (const auto& name : schNets) {
    if (layNets.find(name) == layNets.end()) continue;
    const auto sc = pinCount(schView, name);
    const auto lc = pinCount(layView, name);
    if (sc != lc) {
      std::ostringstream msg;
      msg << "Pin count mismatch on net \"" << name << "\": schematic=" << sc << ", layout=" << lc;
      result.errors.push_back(msg.str());
    }
  }

  // Compare recognized devices between schematic and layout (E4)
  auto schDevices = recognizeDevices(schView, lib);
  auto layDevices = recognizeDevices(layView, lib);
  result.recognizedDevices = layDevices;

  if (schDevices.size() != layDevices.size()) {
    std::ostringstream msg;
    msg << "Device count mismatch: schematic=" << schDevices.size()
        << ", layout=" << layDevices.size();
    result.errors.push_back(msg.str());
  }

  result.matched = result.errors.empty();
  return result;
}

std::vector<RecognizedDevice> LvsChecker::recognizeDevices(const db::DbView& view,
                                                            const db::DbCellLib& lib) const {
  std::vector<RecognizedDevice> devices;

  // Find poly and diff layers
  db::DbId polyId = db::kInvalidId, diffId = db::kInvalidId;
  for (const auto lid : lib.layerIds()) {
    const auto* layer = lib.findLayer(lid);
    if (!layer) continue;
    if (layer->name() == "poly") polyId = lid;
    if (layer->name() == "diff") diffId = lid;
  }
  if (polyId == db::kInvalidId || diffId == db::kInvalidId) return devices;

  // Collect poly rects and diff rects
  std::vector<std::pair<geom::GeomBox, db::DbId>> polyRects, diffRects;
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& r = static_cast<const db::DbRect*>(s)->box();
    if (s->layerId() == polyId) polyRects.emplace_back(r, sid);
    if (s->layerId() == diffId) diffRects.emplace_back(r, sid);
  }

  // Recognize MOS: overlapping poly on diff = gate
  for (const auto& [polyR, psid] : polyRects) {
    for (const auto& [diffR, dsid] : diffRects) {
      if (!polyR.intersects(diffR)) continue;
      auto overlap = polyR.intersection(diffR);
      if (!overlap) continue;

      RecognizedDevice dev;
      dev.type = "NMOS"; // default
      dev.boundingBox = diffR;

      // Extract W and L from geometry
      geom::DbUnit L = overlap->width();  // gate length = poly width over diff
      geom::DbUnit W = overlap->height(); // gate width = diff width under poly
      if (L > W) std::swap(L, W); // ensure W >= L

      dev.params["W"] = static_cast<double>(W);
      dev.params["L"] = static_cast<double>(L);
      dev.params["fingers"] = 1;

      devices.push_back(std::move(dev));
    }
  }

  return devices;
}

}  // namespace aurora::drc_lvs
