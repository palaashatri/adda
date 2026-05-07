#include "drc_lvs/DrcEngine.h"

#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomOps.h"
#include "geom/GeomPolygon.h"
#include "tech/TechDatabase.h"

#include <map>
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
    case db::DbShapeKind::Text:
      return {};
  }
  return {};
}

void DrcEngine::checkMinWidth(const db::DbView& view, const db::DbCellLib& lib,
                               std::vector<DrcViolation>& out) const {
  for (const auto shapeId : view.shapeIds()) {
    const auto* shape = view.findShape(shapeId);
    if (!shape) continue;
    if (shape->kind() == db::DbShapeKind::Text) continue;

    const auto* layer = lib.findLayer(shape->layerId());
    if (!layer) continue;

    const geom::DbUnit minW = tech_.defaultWidthForLayer(layer->name());
    if (minW <= 0) continue;

    const geom::GeomBox box = shapeBoundingBox(*shape);
    if (box.empty()) continue;

    if (!geom::meetsMinimumWidth(box, minW)) {
      std::ostringstream msg;
      msg << "Min-width violation on layer " << layer->name()
          << ": shape width " << std::min(box.width(), box.height())
          << " < " << minW;
      out.push_back({DrcViolationType::MinWidth, layer->name(), msg.str(), box});
    }
  }
}

void DrcEngine::checkMinSpacing(const db::DbView& view, const db::DbCellLib& lib,
                                 std::vector<DrcViolation>& out) const {
  // Collect bounding boxes per layer
  std::map<db::DbId, std::vector<geom::GeomBox>> byLayer;
  for (const auto shapeId : view.shapeIds()) {
    const auto* shape = view.findShape(shapeId);
    if (!shape) continue;
    if (shape->kind() == db::DbShapeKind::Text) continue;
    const geom::GeomBox box = shapeBoundingBox(*shape);
    if (!box.empty()) byLayer[shape->layerId()].push_back(box);
  }

  for (const auto& [layerId, boxes] : byLayer) {
    const auto* layer = lib.findLayer(layerId);
    if (!layer) continue;

    const geom::DbUnit minSp = tech_.defaultSpacingForLayer(layer->name());
    if (minSp <= 0) continue;

    for (std::size_t i = 0; i < boxes.size(); ++i) {
      for (std::size_t j = i + 1; j < boxes.size(); ++j) {
        if (!geom::meetsMinimumSpacing(boxes[i], boxes[j], minSp)) {
          // Report violation at the bounding box of the two shapes
          const geom::GeomBox loc{
              std::min(boxes[i].left(),   boxes[j].left()),
              std::min(boxes[i].bottom(), boxes[j].bottom()),
              std::max(boxes[i].right(),  boxes[j].right()),
              std::max(boxes[i].top(),    boxes[j].top())};
          std::ostringstream msg;
          msg << "Min-spacing violation on layer " << layer->name()
              << ": distance < " << minSp;
          out.push_back({DrcViolationType::MinSpacing, layer->name(), msg.str(), loc});
        }
      }
    }
  }
}

void DrcEngine::checkNonManhattan(const db::DbView& view,
                                   std::vector<DrcViolation>& out) const {
  for (const auto shapeId : view.shapeIds()) {
    const auto* shape = view.findShape(shapeId);
    if (!shape || shape->kind() != db::DbShapeKind::Polygon) continue;
    const auto& poly = static_cast<const db::DbPolygon&>(*shape).polygon();
    if (!geom::isManhattan(poly)) {
      out.push_back({DrcViolationType::NonManhattan, "",
                     "Non-Manhattan polygon shape", poly.boundingBox()});
    }
  }
}

std::vector<DrcViolation> DrcEngine::run(const db::DbView& view,
                                          const db::DbCellLib& lib) const {
  std::vector<DrcViolation> violations;
  checkMinWidth(view, lib, violations);
  checkMinSpacing(view, lib, violations);
  checkNonManhattan(view, violations);
  return violations;
}

}  // namespace aurora::drc_lvs
