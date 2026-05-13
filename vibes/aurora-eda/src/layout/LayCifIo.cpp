#include "layout/LayCifIo.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>

namespace aurora::layout {

bool LayCifIo::write(const db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ofstream o(path);
  if (!o) return false;
  o << "(CIF written by aurora-eda);\n";

  int sym = 1;
  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    const auto* view = cell->findView(db::DbViewType::Layout);
    if (!view) continue;

    o << "DS " << sym << " 1 1;\n";
    o << "9 " << cell->name() << ";\n";

    db::DbId currentLayer = db::kInvalidId;
    for (auto sid : view->shapeIds()) {
      const auto* s = view->findShape(sid);
      if (!s || s->kind() != db::DbShapeKind::Rect) continue;
      if (s->layerId() != currentLayer) {
        const auto* l = lib.findLayer(s->layerId());
        if (l) o << "L " << l->name() << ";\n";
        currentLayer = s->layerId();
      }
      const auto& b = static_cast<const db::DbRect*>(s)->box();
      o << "B " << b.width() << " " << b.height() << " "
        << (b.left() + b.width() / 2) << " " << (b.bottom() + b.height() / 2) << ";\n";
    }
    o << "DF;\n";
    sym++;
  }
  o << "C 1;\nE\n";
  return true;
}

bool LayCifIo::read(db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ifstream in(path);
  if (!in) return false;
  std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

  std::map<std::string, db::DbId> layerCache;
  auto getLayer = [&](const std::string& name) {
    auto it = layerCache.find(name);
    if (it != layerCache.end()) return it->second;
    auto& l = lib.createLayer(name, "drawing");
    layerCache[name] = l.id();
    return l.id();
  };

  db::DbCell* cell = nullptr;
  db::DbView* view = nullptr;
  db::DbId currentLayer = db::kInvalidId;

  std::stringstream ss(text);
  std::string tok;
  while (ss >> tok) {
    if (tok == "DS") {
      int sym; ss >> sym;
      std::string a, b; ss >> a >> b;
      std::string name = "cif_" + std::to_string(sym);
      cell = lib.findCell(name);
      if (!cell) cell = &lib.createCell(name);
      view = &cell->createView(db::DbViewType::Layout);
    } else if (tok == "9") {
      std::string name; std::getline(ss, name, ';');
      // trim
      while (!name.empty() && std::isspace((unsigned char)name.front())) name.erase(name.begin());
      while (!name.empty() && std::isspace((unsigned char)name.back())) name.pop_back();
      if (cell && !name.empty()) {
        // Rename: rebuild not needed; we already have a cell. Just note.
      }
    } else if (tok == "L") {
      std::string name; std::getline(ss, name, ';');
      while (!name.empty() && std::isspace((unsigned char)name.front())) name.erase(name.begin());
      currentLayer = getLayer(name);
    } else if (tok == "B" && view) {
      long long w, h, cx, cy;
      ss >> w >> h >> cx >> cy;
      ss.ignore(256, ';');
      if (currentLayer == db::kInvalidId) currentLayer = getLayer("default");
      (void)view->createRect(currentLayer,
          geom::GeomBox{cx - w/2, cy - h/2, cx + w/2, cy + h/2});
    } else if (tok == "DF;" || tok == "DF") {
      cell = nullptr;
      view = nullptr;
      currentLayer = db::kInvalidId;
    } else if (tok == "E") {
      break;
    }
  }
  return true;
}

}  // namespace aurora::layout
