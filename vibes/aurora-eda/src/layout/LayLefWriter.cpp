#include "layout/LayLefWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbLayer.h"
#include "db/DbPin.h"
#include "db/DbShape.h"
#include "db/DbTypes.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <set>

namespace aurora::layout {

static void writeRect(std::ostream& o, const geom::GeomBox& box, double dbuPerMicron) {
  auto toU = [&](long long v) { return static_cast<double>(v) / dbuPerMicron; };
  o << "      RECT " << toU(box.left()) << " " << toU(box.bottom()) << " "
                      << toU(box.right()) << " " << toU(box.top()) << " ;\n";
}

bool LayLefWriter::write(const db::DbCellLib& lib, const std::filesystem::path& path,
                          double dbuPerMicron) const {
  std::ofstream o(path);
  if (!o) return false;

  o << "VERSION 5.8 ;\n";
  o << "BUSBITCHARS \"<>\" ;\n";
  o << "DIVIDERCHAR \"/\" ;\n\n";

  // Write layer-to-LEF layer mapping based on GDS numbers
  o << "LAYER\n";
  std::map<int, std::string> gdsToLayerName;
  for (const auto lid : lib.layerIds()) {
    const auto* l = lib.findLayer(lid);
    if (!l || l->gdsLayer() < 0) continue;
    const std::string lefName = l->name();
    o << "  " << lefName << " TYPE MASTERSLICE ;\n";
    gdsToLayerName[l->gdsLayer()] = lefName;
  }
  o << "END LAYER\n\n";

  // One MACRO per cell with a layout view
  for (const auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    const auto* view = cell->findView(db::DbViewType::Layout);
    if (!view) continue;

    o << "MACRO " << cell->name() << "\n";

    // Compute bounding box of all shapes
    geom::DbUnit minX = 0, minY = 0, maxX = 0, maxY = 0;
    bool hasGeom = false;
    for (const auto sid : view->shapeIds()) {
      const auto* s = view->findShape(sid);
      if (!s) continue;
      geom::GeomBox bb;
      switch (s->kind()) {
        case db::DbShapeKind::Rect:
          bb = static_cast<const db::DbRect*>(s)->box(); break;
        default: continue;
      }
      if (!hasGeom) {
        minX = bb.left(); minY = bb.bottom(); maxX = bb.right(); maxY = bb.top();
        hasGeom = true;
      } else {
        minX = std::min(minX, bb.left());
        minY = std::min(minY, bb.bottom());
        maxX = std::max(maxX, bb.right());
        maxY = std::max(maxY, bb.top());
      }
    }

    if (!hasGeom) {
      o << "  CLASS CORE ;\n";
      o << "  ORIGIN 0 0 ;\n";
      o << "  SIZE 0 BY 0 ;\n";
      o << "END " << cell->name() << "\n\n";
      continue;
    }

    const double w = static_cast<double>(maxX - minX) / dbuPerMicron;
    const double h = static_cast<double>(maxY - minY) / dbuPerMicron;

    o << "  CLASS CORE ;\n";
    o << "  ORIGIN 0 0 ;\n";
    o << "  SIZE " << w << " BY " << h << " ;\n";

    // Pins: scan shapes in the layout that correspond to pins
    for (const auto pid : view->pinIds()) {
      const auto* pin = view->findPin(pid);
      if (!pin) continue;
      o << "  PIN " << pin->name() << "\n";
      std::string_view dir;
      switch (pin->direction()) {
        case db::DbPinDirection::Input:  dir = "INPUT";  break;
        case db::DbPinDirection::Output: dir = "OUTPUT"; break;
        case db::DbPinDirection::InOut:  dir = "INOUT";  break;
        default:                         dir = "SIGNAL"; break;
      }
      o << "    DIRECTION " << dir << " ;\n";

      // Pin geometry is on the pin's shape IDs
      for (const auto psid : pin->shapeIds()) {
        const auto* ps = view->findShape(psid);
        if (!ps || ps->kind() != db::DbShapeKind::Rect) continue;
        const auto& box = static_cast<const db::DbRect*>(ps)->box();
        const auto* layer = lib.findLayer(ps->layerId());
        if (!layer) continue;
        o << "    PORT\n";
        o << "      LAYER " << layer->name() << " ;\n";
        writeRect(o, box, dbuPerMicron);
        o << "    END\n";
      }
      o << "  END " << pin->name() << "\n";
    }

    // Obstructions: write shape geometry as OBS
    o << "  OBS\n";
    for (const auto sid : view->shapeIds()) {
      const auto* s = view->findShape(sid);
      if (!s || s->kind() != db::DbShapeKind::Rect) continue;
      const auto& box = static_cast<const db::DbRect*>(s)->box();
      const auto* layer = lib.findLayer(s->layerId());
      if (!layer) continue;
      o << "    LAYER " << layer->name() << " ;\n";
      writeRect(o, box, dbuPerMicron);
    }
    o << "  END\n";

    o << "END " << cell->name() << "\n\n";
  }

  return o.good();
}

}  // namespace aurora::layout
