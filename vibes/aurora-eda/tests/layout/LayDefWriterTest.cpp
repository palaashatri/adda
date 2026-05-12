#include "layout/LayDefWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbLayer.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbTypes.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <cassert>
#include <filesystem>
#include <fstream>

static aurora::db::DbCellLib buildLib() {
  aurora::db::DbCellLib lib("testdesign");

  auto& met1 = lib.createLayer("metal1", "drawing");
  met1.setGdsMapping(68, 0);

  // Sub-cell (standard cell)
  auto& inv = lib.createCell("INV");
  auto& lvInv = inv.createView(aurora::db::DbViewType::Layout);
  (void)lvInv.createRect(met1.id(), aurora::geom::GeomBox{0, 0, 1000, 2000});
  (void)lvInv.createPin("A", aurora::db::DbPinDirection::Input);
  (void)lvInv.createPin("Y", aurora::db::DbPinDirection::Output);

  // Top cell
  auto& top = lib.createCell("TOP");
  auto& lv = top.createView(aurora::db::DbViewType::Layout);
  auto& i1 = lv.createInstance("I1", inv.id(), {5000, 6000, 0, false});
  auto& i2 = lv.createInstance("I2", inv.id(), {15000, 6000, 0, false});

  // Nets
  auto& netA = lv.createNet("A");
  auto& netMid = lv.createNet("MID");
  auto& netY = lv.createNet("Y");

  // Pins (connect nets to instance pins)
  (void)lv.createPin("A", aurora::db::DbPinDirection::Input, netA.id());
  (void)lv.createPin("Y", aurora::db::DbPinDirection::Output, netY.id());
  (void)lv.createPin("A", aurora::db::DbPinDirection::Input, netMid.id(), i1.id());
  (void)lv.createPin("Y", aurora::db::DbPinDirection::Output, netY.id(), i1.id());
  (void)lv.createPin("A", aurora::db::DbPinDirection::Input, netMid.id(), i2.id());
  (void)lv.createPin("Y", aurora::db::DbPinDirection::Output, netMid.id(), i2.id());

  return lib;
}

static void testDefOutput() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_def_test.def";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayDefWriter{}.write(lib, path);
  assert(ok && "write must succeed");

  std::ifstream f(path);
  std::string content((std::istreambuf_iterator<char>(f)), {});
  std::filesystem::remove(path);

  assert(content.find("DESIGN testdesign") != std::string::npos && "must have design name");
  assert(content.find("COMPONENTS") != std::string::npos && "must have COMPONENTS");
  assert(content.find("I1 INV") != std::string::npos && "must have I1 instance");
  assert(content.find("I2 INV") != std::string::npos && "must have I2 instance");
  assert(content.find("PLACED") != std::string::npos && "must have PLACED");
  assert(content.find("NETS") != std::string::npos && "must have NETS");
  assert(content.find("MID") != std::string::npos && "must have MID net");
}

int main() {
  testDefOutput();
  return 0;
}
