#include "netlist/VerilogGenerator.h"
#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbView.h"

#include <algorithm>
#include <sstream>

namespace aurora::netlist {

std::string VerilogGenerator::generateVerilog(const db::DbCellLib& lib, const db::DbCell& cell,
                                               const db::DbView& view) const {
  std::ostringstream out;

  out << "// aurora-eda  cell=" << cell.name() << "  view=" << db::toString(view.type()) << "\n";

  // Collect top-level port names (pins with no instance)
  std::vector<std::string> topPorts;
  for (const auto pid : view.pinIds()) {
    const auto* pin = view.findPin(pid);
    if (pin && pin->instanceId() == db::kInvalidId)
      topPorts.push_back(pin->name());
  }

  // Module declaration
  out << "module " << cell.name() << " (";
  for (std::size_t i = 0; i < topPorts.size(); ++i) {
    if (i > 0) out << ", ";
    out << topPorts[i];
  }
  out << ");\n\n";

  // Port directions
  for (const auto pid : view.pinIds()) {
    const auto* pin = view.findPin(pid);
    if (!pin || pin->instanceId() != db::kInvalidId) continue;
    std::string_view dirStr;
    switch (pin->direction()) {
      case db::DbPinDirection::Input:  dirStr = "input";  break;
      case db::DbPinDirection::Output: dirStr = "output"; break;
      case db::DbPinDirection::InOut:  dirStr = "inout";  break;
      default:                         dirStr = "wire";   break;
    }
    out << "  " << dirStr << " " << pin->name() << ";\n";
  }
  out << "\n";

  // Wire declarations for all nets (skip those that are also ports)
  for (const auto nid : view.netIds()) {
    const auto* net = view.findNet(nid);
    if (!net) continue;
    // Skip if this net name matches a port name
    if (std::find(topPorts.begin(), topPorts.end(), net->name()) != topPorts.end())
      continue;
    out << "  wire " << net->name() << ";\n";
  }
  if (!view.netIds().empty()) out << "\n";

  // Helper: resolve net name for a pin
  auto netNameForPin = [&](db::DbId instanceId, const std::string& pinName) -> std::string {
    for (const auto* ip : view.findInstancePins(instanceId))
      if (ip->name() == pinName) {
        if (ip->netId() != db::kInvalidId) {
          const auto* n = view.findNet(ip->netId());
          if (n) return n->name();
        }
        return "";
      }
    return "";
  };

  // Instance instantiations
  for (const auto iid : view.instanceIds()) {
    const auto* inst = view.findInstance(iid);
    if (!inst) continue;

    const auto* master = lib.findCellById(inst->masterCellId());
    if (!master) continue;

    out << "  " << master->name() << " " << inst->name() << " (";
    bool first = true;

    // Find the symbol view to get pin order
    const auto* symView = master->findView(db::DbViewType::Symbol);
    if (symView) {
      for (const auto mpid : symView->pinIds()) {
        const auto* mpin = symView->findPin(mpid);
        if (!mpin) continue;
        std::string connected = netNameForPin(inst->id(), mpin->name());
        if (!first) out << ", ";
        out << "." << mpin->name() << "(" << connected << ")";
        first = false;
      }
    }
    out << ");\n";
  }

  out << "\nendmodule\n";
  return out.str();
}

}  // namespace aurora::netlist
