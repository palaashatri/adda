#include "layout/LayDxfWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <fstream>

namespace aurora::layout {

bool LayDxfWriter::write(const db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ofstream o(path);
  if (!o) return false;

  o << "0\nSECTION\n2\nHEADER\n0\nENDSEC\n";
  o << "0\nSECTION\n2\nTABLES\n";
  o << "0\nTABLE\n2\nLAYER\n";
  for (auto lid : lib.layerIds()) {
    const auto* l = lib.findLayer(lid);
    if (!l) continue;
    o << "0\nLAYER\n2\n" << l->name() << "\n70\n0\n62\n7\n6\nCONTINUOUS\n";
  }
  o << "0\nENDTAB\n0\nENDSEC\n";

  o << "0\nSECTION\n2\nENTITIES\n";
  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    const auto* view = cell->findView(db::DbViewType::Layout);
    if (!view) continue;
    for (auto sid : view->shapeIds()) {
      const auto* s = view->findShape(sid);
      if (!s || s->kind() != db::DbShapeKind::Rect) continue;
      const auto& b = static_cast<const db::DbRect*>(s)->box();
      const auto* l = lib.findLayer(s->layerId());
      const std::string layerName = l ? l->name() : "0";
      o << "0\nLWPOLYLINE\n8\n" << layerName << "\n90\n4\n70\n1\n";
      o << "10\n" << b.left() << "\n20\n" << b.bottom() << "\n";
      o << "10\n" << b.right() << "\n20\n" << b.bottom() << "\n";
      o << "10\n" << b.right() << "\n20\n" << b.top() << "\n";
      o << "10\n" << b.left() << "\n20\n" << b.top() << "\n";
    }
  }
  o << "0\nENDSEC\n0\nEOF\n";
  return true;
}

}  // namespace aurora::layout
