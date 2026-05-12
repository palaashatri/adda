#include "drc_lvs/DrcEngine.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbLayer.h"
#include "db/DbPin.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomOps.h"
#include "geom/GeomPolygon.h"
#include "tech/TechDatabase.h"

#include <map>
#include <set>
#include <sstream>

namespace aurora::drc_lvs {

DrcEngine::DrcEngine(const tech::TechDatabase& tech) : tech_(tech) {}

static geom::GeomBox shapeBoundingBox(const db::DbShape& shape) {
  switch (shape.kind()) {
    case db::DbShapeKind::Rect:
      return static_cast<const db::DbRect&>(shape).box();
    case db::DbShapeKind::Polygon:
      return static_cast<const db::DbPolygon&>(shape).polygon().boundingBox();
    case db::DbShapeKind::Path: {
      const auto& path = static_cast<const db::DbPath&>(shape).path();
      if (path.empty()) return {};
      const auto& pts = path.points();
      geom::DbUnit minX = pts[0].x, maxX = pts[0].x;
      geom::DbUnit minY = pts[0].y, maxY = pts[0].y;
      for (const auto& p : pts) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
      }
      const geom::DbUnit hw = path.width() / 2;
      return {minX - hw, minY - hw, maxX + hw, maxY + hw};
    }
    default: return {};
  }
}

void DrcEngine::checkMinWidth(const db::DbView& view, const db::DbCellLib& lib,
                               std::vector<DrcViolation>& out) const {
  for (const auto shapeId : view.shapeIds()) {
    const auto* shape = view.findShape(shapeId);
    if (!shape || shape->kind() == db::DbShapeKind::Text) continue;
    const auto* layer = lib.findLayer(shape->layerId());
    if (!layer) continue;
    const geom::DbUnit minW = tech_.defaultWidthForLayer(layer->name());
    if (minW <= 0) continue;
    const auto box = shapeBoundingBox(*shape);
    if (box.empty()) continue;
    if (!geom::meetsMinimumWidth(box, minW)) {
      std::ostringstream msg;
      msg << "Min-width violation on " << layer->name()
          << ": " << std::min(box.width(), box.height()) << " < " << minW;
      out.push_back({DrcViolationType::MinWidth, layer->name(), msg.str(), box});
    }
  }
}

void DrcEngine::checkMinSpacing(const db::DbView& view, const db::DbCellLib& lib,
                                 std::vector<DrcViolation>& out) const {
  std::map<db::DbId, std::vector<geom::GeomBox>> byLayer;
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() == db::DbShapeKind::Text) continue;
    auto box = shapeBoundingBox(*s);
    if (!box.empty()) byLayer[s->layerId()].push_back(box);
  }
  for (const auto& [lid, boxes] : byLayer) {
    const auto* layer = lib.findLayer(lid);
    if (!layer) continue;
    const auto minSp = tech_.defaultSpacingForLayer(layer->name());
    if (minSp <= 0) continue;
    for (std::size_t i = 0; i < boxes.size(); ++i)
      for (std::size_t j = i + 1; j < boxes.size(); ++j)
        if (!geom::meetsMinimumSpacing(boxes[i], boxes[j], minSp)) {
          geom::GeomBox loc{std::min(boxes[i].left(),boxes[j].left()),
                            std::min(boxes[i].bottom(),boxes[j].bottom()),
                            std::max(boxes[i].right(),boxes[j].right()),
                            std::max(boxes[i].top(),boxes[j].top())};
          std::ostringstream msg;
          msg << "Spacing violation on " << layer->name() << " < " << minSp;
          out.push_back({DrcViolationType::MinSpacing, layer->name(), msg.str(), loc});
        }
  }
}

void DrcEngine::checkNonManhattan(const db::DbView& view, std::vector<DrcViolation>& out) const {
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Polygon) continue;
    const auto& poly = static_cast<const db::DbPolygon&>(*s).polygon();
    if (!geom::isManhattan(poly))
      out.push_back({DrcViolationType::NonManhattan, "", "Non-Manhattan polygon", poly.boundingBox()});
  }
}

// E10: Electrical rule checks (ERC)
void DrcEngine::checkErc(const db::DbView& view, const db::DbCellLib& lib,
                          std::vector<DrcViolation>& out) const {
  // Check floating nets (nets with 0 pins)
  for (const auto nid : view.netIds()) {
    const auto* net = view.findNet(nid);
    if (!net) continue;
    if (net->pinIds().empty()) {
      out.push_back({DrcViolationType::ERC, "",
                     "Floating net: " + net->name(), {}});
    }
  }

  // Check for multiple drivers (output pins on the same net)
  std::map<std::string, int> driverCount;
  for (const auto pid : view.pinIds()) {
    const auto* pin = view.findPin(pid);
    if (!pin || pin->direction() != db::DbPinDirection::Output) continue;
    const auto* net = view.findNet(pin->netId());
    if (!net) continue;
    driverCount[net->name()]++;
  }
  for (const auto& [netName, count] : driverCount) {
    if (count > 1) {
      out.push_back({DrcViolationType::ERC, "",
                     "Multiple drivers on net: " + netName, {}});
    }
  }

  // Check for floating pins (pins with no net)
  for (const auto pid : view.pinIds()) {
    const auto* pin = view.findPin(pid);
    if (!pin || pin->netId() != db::kInvalidId) continue;
    out.push_back({DrcViolationType::ERC, "",
                   "Unconnected pin: " + pin->name(), {}});
  }
}

// E8: Antenna checking (metal area / gate area ratio)
void DrcEngine::checkAntenna(const db::DbView& view, const db::DbCellLib& lib,
                              std::vector<DrcViolation>& out) const {
  // Collect metal area per layer, and gate (poly) area
  std::map<db::DbId, double> metalArea; // layerId → total area
  double gateArea = 0;
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s) continue;
    auto box = shapeBoundingBox(*s);
    if (box.empty()) continue;
    double area = static_cast<double>(box.width()) * box.height();
    const auto* layer = lib.findLayer(s->layerId());
    if (!layer) continue;
    if (layer->name().find("metal") != std::string::npos || layer->name().find("Metal") != std::string::npos)
      metalArea[s->layerId()] += area;
    if (layer->name() == "poly")
      gateArea += area;
  }

  if (gateArea > 0) {
    for (const auto& [lid, mArea] : metalArea) {
      double ratio = mArea / gateArea;
      if (ratio > 1000) { // typical antenna ratio limit
        const auto* layer = lib.findLayer(lid);
        std::ostringstream msg;
        msg << "Antenna violation on " << (layer ? layer->name() : "?")
            << ": ratio " << ratio << " > 1000";
        out.push_back({DrcViolationType::Antenna,
                      layer ? layer->name() : "", msg.str(), {}});
      }
    }
  }
}

// E9: Density checking
void DrcEngine::checkDensity(const db::DbView& view, const db::DbCellLib& lib,
                              std::vector<DrcViolation>& out) const {
  constexpr double minDensity = 0.20; // 20% minimum
  constexpr double maxDensity = 0.80; // 80% maximum
  constexpr geom::DbUnit binSize = 100000; // 100µm bins

  // Compute overall bounds
  geom::DbUnit minX = 0, minY = 0, maxX = 0, maxY = 0;
  bool first = true;
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s) continue;
    auto box = shapeBoundingBox(*s);
    if (box.empty()) continue;
    if (first) { minX = box.left(); maxX = box.right(); minY = box.bottom(); maxY = box.top(); first = false; }
    else {
      minX = std::min(minX, box.left()); maxX = std::max(maxX, box.right());
      minY = std::min(minY, box.bottom()); maxY = std::max(maxY, box.top());
    }
  }
  if (first) return;

  // Bin check per layer
  for (const auto lid : lib.layerIds()) {
    const auto* layer = lib.findLayer(lid);
    if (!layer) continue;
    const auto* techInfo = tech_.findLayerByName(layer->name());
    if (!techInfo) continue;

    // Collect shapes on this layer
    std::vector<geom::GeomBox> layerBoxes;
    for (const auto sid : view.shapeIds()) {
      const auto* s = view.findShape(sid);
      if (!s || s->layerId() != lid) continue;
      auto box = shapeBoundingBox(*s);
      if (!box.empty()) layerBoxes.push_back(box);
    }
    if (layerBoxes.empty()) continue;

    // Sample density at center bin
    geom::DbUnit cx = (minX + maxX) / 2, cy = (minY + maxY) / 2;
    geom::GeomBox sampleBin{cx - binSize / 2, cy - binSize / 2, cx + binSize / 2, cy + binSize / 2};
    double filled = 0;
    for (const auto& b : layerBoxes) {
      if (auto inter = sampleBin.intersection(b)) {
        filled += static_cast<double>(inter->width()) * inter->height();
      }
    }
    double binArea = static_cast<double>(binSize) * binSize;
    double density = binArea > 0 ? filled / binArea : 0;

    if (density < minDensity && density > 0) {
      std::ostringstream msg;
      msg << "Low density on " << layer->name() << ": " << (density * 100) << "% < "
          << (minDensity * 100) << "%";
      out.push_back({DrcViolationType::Density, layer->name(), msg.str(), sampleBin});
    }
    if (density > maxDensity) {
      std::ostringstream msg;
      msg << "High density on " << layer->name() << ": " << (density * 100) << "% > "
          << (maxDensity * 100) << "%";
      out.push_back({DrcViolationType::Density, layer->name(), msg.str(), sampleBin});
    }
  }
}

std::vector<DrcViolation> DrcEngine::run(const db::DbView& view,
                                          const db::DbCellLib& lib,
                                          const DrcOptions& opts) const {
  std::vector<DrcViolation> violations;

  if (opts.checkMinWidth) checkMinWidth(view, lib, violations);
  if (opts.checkMinSpacing) checkMinSpacing(view, lib, violations);
  if (opts.checkNonManhattan) checkNonManhattan(view, violations);
  if (opts.checkErc) checkErc(view, lib, violations);
  if (opts.checkAntenna) checkAntenna(view, lib, violations);
  if (opts.checkDensity) checkDensity(view, lib, violations);

  return violations;
}

}  // namespace aurora::drc_lvs
