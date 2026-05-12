#include "netlist/VerilogGenerator.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbTypes.h"
#include "db/DbView.h"

#include <cassert>

static aurora::db::DbCellLib buildLib() {
  aurora::db::DbCellLib lib("worklib");

  // Create a sub-cell (INV)
  auto& inv = lib.createCell("INV");
  auto& svInv = inv.createView(aurora::db::DbViewType::Symbol);
  (void)svInv.createPin("A", aurora::db::DbPinDirection::Input);
  (void)svInv.createPin("Y", aurora::db::DbPinDirection::Output);

  // Create top cell
  auto& top = lib.createCell("TOP");
  auto& sv = top.createView(aurora::db::DbViewType::Schematic);

  // Nets
  auto& netIn = sv.createNet("IN");
  auto& netOut = sv.createNet("OUT");
  auto& mid = sv.createNet("MID");

  // Ports (associate pins with nets)
  auto& pinIn = sv.createPin("IN", aurora::db::DbPinDirection::Input, netIn.id());
  auto& pinOut = sv.createPin("OUT", aurora::db::DbPinDirection::Output, netOut.id());

  // Instance
  auto& inst = sv.createInstance("X1", inv.id());
  (void)sv.createPin("A", aurora::db::DbPinDirection::Input, mid.id(), inst.id());
  (void)sv.createPin("Y", aurora::db::DbPinDirection::Output, netOut.id(), inst.id());

  return lib;
}

static void testVerilogOutput() {
  auto lib = buildLib();
  const auto* cell = lib.findCell("TOP");
  assert(cell != nullptr);
  const auto* view = cell->findView(aurora::db::DbViewType::Schematic);
  assert(view != nullptr);

  aurora::netlist::VerilogGenerator gen;
  const std::string vlog = gen.generateVerilog(lib, *cell, *view);

  // Basic structure checks
  assert(vlog.find("module TOP") != std::string::npos && "must have module TOP");
  assert(vlog.find("endmodule") != std::string::npos && "must have endmodule");
  assert(vlog.find("input IN") != std::string::npos && "must have input port");
  assert(vlog.find("output OUT") != std::string::npos && "must have output port");
  assert(vlog.find("wire MID") != std::string::npos && "must have wire MID");
  assert(vlog.find("INV X1") != std::string::npos && "must instantiate INV as X1");
  assert(vlog.find(".A(MID)") != std::string::npos && "must connect A to MID");
  assert(vlog.find(".Y(OUT)") != std::string::npos && "must connect Y to OUT");
}

int main() {
  testVerilogOutput();
  return 0;
}
