#include "layout/LayLefWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbPin.h"
#include "db/DbShape.h"
#include "db/DbTypes.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "geom/GeomPoint.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <vector>

static aurora::db::DbCellLib buildLib() {
  aurora::db::DbCellLib lib("stdcells");

  auto& met1 = lib.createLayer("metal1", "drawing");
  met1.setColor("#5080d0");
  met1.setGdsMapping(68, 0);

  auto& poly = lib.createLayer("poly", "drawing");
  poly.setColor("#e05050");
  poly.setGdsMapping(46, 0);

  auto& diff = lib.createLayer("diff", "drawing");
  diff.setColor("#3fbf7f");
  diff.setGdsMapping(22, 0);

  // AND2 cell
  auto& cell = lib.createCell("AND2");
  auto& lv = cell.createView(aurora::db::DbViewType::Layout);
  using B = aurora::geom::GeomBox;
  (void)lv.createRect(diff.id(), B{0, 0, 2000, 3000});
  (void)lv.createRect(poly.id(), B{800, -200, 1200, 3200});

  // Pins
  auto& pinA = lv.createPin("A", aurora::db::DbPinDirection::Input);
  auto& aShape = lv.createRect(met1.id(), B{100, 100, 400, 400});
  pinA.addShape(aShape.id());
  (void)lv.createNet("A"); // connect net
  auto& pinY = lv.createPin("Y", aurora::db::DbPinDirection::Output);
  auto& yShape = lv.createRect(met1.id(), B{1600, 2600, 1900, 2900});
  pinY.addShape(yShape.id());
  (void)lv.createNet("Y");

  return lib;
}

static void testLefOutput() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_lef_test.lef";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayLefWriter{}.write(lib, path);
  assert(ok && "write must succeed");
  assert(std::filesystem::exists(path) && "file must exist");

  std::ifstream f(path);
  std::string content((std::istreambuf_iterator<char>(f)), {});
  std::filesystem::remove(path);

  assert(content.find("VERSION 5.8") != std::string::npos && "must have version");
  assert(content.find("MACRO AND2") != std::string::npos && "must have AND2 macro");
  assert(content.find("END AND2") != std::string::npos && "must end AND2 macro");
  assert(content.find("PIN A") != std::string::npos && "must have pin A");
  assert(content.find("PIN Y") != std::string::npos && "must have pin Y");
  assert(content.find("DIRECTION INPUT") != std::string::npos && "A must be input");
  assert(content.find("DIRECTION OUTPUT") != std::string::npos && "Y must be output");
  assert(content.find("OBS") != std::string::npos && "must have OBS section");
  assert(content.find("LAYER metal1") != std::string::npos && "must reference metal1");
  assert(content.find("LAYER poly") != std::string::npos && "must reference poly");
}

int main() {
  testLefOutput();
  return 0;
}
