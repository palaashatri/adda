#include "layout/LayLefReader.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbPin.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <fstream>
#include <sstream>
#include <string>

namespace aurora::layout {

namespace {
double toUnits(double v, double scale) { return v * scale; }

db::DbId findOrAddLayer(db::DbCellLib& lib, const std::string& name) {
  for (auto id : lib.layerIds()) {
    if (auto* l = lib.findLayer(id); l && l->name() == name) return id;
  }
  auto& l = lib.createLayer(name, "drawing");
  return l.id();
}
}  // namespace

bool LayLefReader::read(db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ifstream in(path);
  if (!in) return false;

  std::string token;
  std::string currentMacro;
  db::DbCell* cell = nullptr;
  db::DbView* view = nullptr;
  double units = 1000.0;  // DBU per micron default

  std::string currentLayer;
  std::string currentPin;
  db::DbPinDirection pinDir = db::DbPinDirection::InOut;

  while (in >> token) {
    if (token == "UNITS") {
      std::string a, b, c, d;
      in >> a >> b >> c;  // e.g. DATABASE MICRONS 1000
      if (a == "DATABASE" && b == "MICRONS") {
        try { units = std::stod(c); } catch (...) {}
      }
    } else if (token == "MACRO") {
      in >> currentMacro;
      cell = lib.findCell(currentMacro);
      if (!cell) cell = &lib.createCell(currentMacro);
      view = &cell->createView(db::DbViewType::Layout);
    } else if (token == "END") {
      std::string n; in >> n;
      if (n == currentMacro) { cell = nullptr; view = nullptr; currentMacro.clear(); }
      else if (n == currentPin) { currentPin.clear(); }
    } else if (token == "SIZE" && view) {
      double w, h; std::string by, semi;
      in >> w >> by >> h >> semi;
      auto lid = findOrAddLayer(lib, "outline");
      (void)view->createRect(lid, geom::GeomBox{0, 0, (long long)toUnits(w, units),
                                                (long long)toUnits(h, units)});
    } else if (token == "PIN" && view) {
      in >> currentPin;
      pinDir = db::DbPinDirection::InOut;
    } else if (token == "DIRECTION" && view && !currentPin.empty()) {
      std::string d; in >> d;
      if (d == "INPUT") pinDir = db::DbPinDirection::Input;
      else if (d == "OUTPUT") pinDir = db::DbPinDirection::Output;
      else pinDir = db::DbPinDirection::InOut;
      (void)view->createPin(currentPin, pinDir);
    } else if (token == "LAYER" && view) {
      in >> currentLayer;
    } else if (token == "RECT" && view && !currentLayer.empty()) {
      double x1, y1, x2, y2;
      in >> x1 >> y1 >> x2 >> y2;
      auto lid = findOrAddLayer(lib, currentLayer);
      (void)view->createRect(lid, geom::GeomBox{(long long)toUnits(x1, units),
                                                (long long)toUnits(y1, units),
                                                (long long)toUnits(x2, units),
                                                (long long)toUnits(y2, units)});
    }
  }
  return true;
}

}  // namespace aurora::layout
