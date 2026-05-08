#include "netlist/NetlistGenerator.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"

#include <sstream>

namespace aurora::netlist {

std::string NetlistGenerator::generateSpice(const db::DbCellLib& lib, const db::DbCell& cell,
                                            const db::DbView& view) const {
  std::ostringstream out;

  out << "* aurora-eda  cell=" << cell.name() << "  view=" << db::toString(view.type()) << "\n";

  // Collect pin names ordered by ID for the port list
  const auto pinIds = view.pinIds();
  std::vector<std::string> topPortNames;
  for (const auto pid : pinIds) {
    const auto* pin = view.findPin(pid);
    if (pin != nullptr && pin->instanceId() == db::kInvalidId) {
      topPortNames.push_back(pin->name());
    }
  }

  // .subckt header
  out << ".subckt " << cell.name();
  for (const auto& port : topPortNames) {
    out << " " << port;
  }
  out << "\n";

  // Net connectivity helper: id → SPICE name
  const auto netName = [&](db::DbId netId) -> std::string {
    if (netId == db::kInvalidId) {
      return "0";
    }
    const auto* net = view.findNet(netId);
    return (net != nullptr) ? net->name() : "0";
  };

  // Emit one X-line per instance
  for (const auto iid : view.instanceIds()) {
    const auto* inst = view.findInstance(iid);
    if (inst == nullptr) {
      continue;
    }

    const auto& instName = inst->name();
    const auto xPrefix = (!instName.empty() && instName[0] == 'X') ? "" : "X";
    out << xPrefix << instName;

    // Resolve nets for each pin of the master
    const auto* masterCell = lib.findCellById(inst->masterCellId());
    if (masterCell) {
      // Find the Symbol view of the master to get pin order
      if (const auto* symbolView = masterCell->findView(db::DbViewType::Symbol)) {
        const auto masterPinIds = symbolView->pinIds();
        // For each master pin, find the corresponding pin in the current view
        for (const auto mpid : masterPinIds) {
          const auto* mpin = symbolView->findPin(mpid);
          if (!mpin) continue;

          // Search for a pin in 'view' that has instanceId == iid AND same name
          std::string connectedNet = "0";
          auto instPins = view.findInstancePins(iid);
          for (const auto* ipin : instPins) {
            if (ipin->name() == mpin->name()) {
              connectedNet = netName(ipin->netId());
              break;
            }
          }
          out << " " << connectedNet;
        }
      }

      out << " " << masterCell->name();
    } else {
      out << " cell_" << inst->masterCellId();
    }

    // Parameters
    for (const auto& [name, value] : inst->parameters()) {
      out << " " << name << "=" << value;
    }
    out << "\n";
  }

  out << ".ends " << cell.name() << "\n";
  return out.str();
}

}  // namespace aurora::netlist
