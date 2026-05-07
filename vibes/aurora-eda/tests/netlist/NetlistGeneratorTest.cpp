#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"
#include "netlist/NetlistGenerator.h"

#include <cassert>
#include <string>

int main() {
  aurora::db::DbCellLib lib{"testlib"};
  auto& cell = lib.createCell("inv");

  auto& view = cell.createView(aurora::db::DbViewType::Schematic);

  auto& vdd = view.createNet("VDD");
  auto& vss = view.createNet("VSS");
  auto& a = view.createNet("A");
  auto& z = view.createNet("Z");

  (void)view.createPin("VDD", aurora::db::DbPinDirection::Passive, vdd.id());
  (void)view.createPin("VSS", aurora::db::DbPinDirection::Passive, vss.id());
  (void)view.createPin("A", aurora::db::DbPinDirection::Input, a.id());
  (void)view.createPin("Z", aurora::db::DbPinDirection::Output, z.id());

  aurora::netlist::NetlistGenerator gen;
  const auto spice = gen.generateSpice(lib, cell, view);

  // Basic structure checks
  assert(spice.find(".subckt inv") != std::string::npos);
  assert(spice.find(".ends inv") != std::string::npos);
  assert(spice.find("VDD") != std::string::npos);
  assert(spice.find("VSS") != std::string::npos);
  assert(spice.find("A") != std::string::npos);
  assert(spice.find("Z") != std::string::npos);

  // Empty-view cell produces valid but empty subckt
  auto& emptyCell = lib.createCell("empty_cell");
  auto& emptyView = emptyCell.createView(aurora::db::DbViewType::Schematic);
  const auto emptySpice = gen.generateSpice(lib, emptyCell, emptyView);
  assert(emptySpice.find(".subckt empty_cell") != std::string::npos);
  assert(emptySpice.find(".ends empty_cell") != std::string::npos);

  return 0;
}
