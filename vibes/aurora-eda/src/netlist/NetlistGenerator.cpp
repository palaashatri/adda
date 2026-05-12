#include "netlist/NetlistGenerator.h"
#include "db/DbCellLib.h"
#include "db/DbConstraint.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"

#include <sstream>

namespace aurora::netlist {

namespace {
const std::string& getOrDefault(const std::map<std::string, std::string>& m,
                                 const std::string& key, const std::string& def) {
  auto it = m.find(key);
  return it != m.end() ? it->second : def;
}
}

std::string NetlistGenerator::generateSpice(const db::DbCellLib& lib, const db::DbCell& cell,
                                            const db::DbView& view) const {
  std::ostringstream out;

  out << "* aurora-eda  cell=" << cell.name() << "  view=" << db::toString(view.type()) << "\n";

  const auto pinIds = view.pinIds();
  std::vector<std::string> topPortNames;
  for (const auto pid : pinIds) {
    const auto* pin = view.findPin(pid);
    if (pin != nullptr && pin->instanceId() == db::kInvalidId)
      topPortNames.push_back(pin->name());
  }

  out << ".subckt " << cell.name();
  for (const auto& port : topPortNames)
    out << " " << port;
  out << "\n";

  const auto netName = [&](db::DbId netId) -> std::string {
    if (netId == db::kInvalidId) return "0";
    const auto* net = view.findNet(netId);
    return (net != nullptr) ? net->name() : "0";
  };

  // Emit stimulus markers as SPICE sources
  int stimIdx = 0;
  for (const auto cid : view.constraintIds()) {
    const auto* con = view.findConstraint(cid);
    if (!con) continue;
    const auto& type = con->type();
    bool isStim = (type == "vdc" || type == "idc" || type == "vpulse" || type == "vsin");
    if (!isStim) continue;

    const auto& props = con->properties();
    const auto& netStr = netName(con->objectIds().empty() ? db::kInvalidId : con->objectIds()[0]);
    const std::string ref = netStr + " 0";
    ++stimIdx;

    if (type == "vdc") {
      out << "V" << stimIdx << " " << ref << " DC "
          << getOrDefault(props, "dc", "0") << "\n";
    } else if (type == "idc") {
      out << "I" << stimIdx << " " << ref << " DC "
          << getOrDefault(props, "dc", "0") << "\n";
    } else if (type == "vpulse") {
      out << "V" << stimIdx << " " << ref << " PULSE("
          << getOrDefault(props, "v1", "0") << " "
          << getOrDefault(props, "v2", "1") << " "
          << getOrDefault(props, "td", "0") << " "
          << getOrDefault(props, "tr", "1n") << " "
          << getOrDefault(props, "tf", "1n") << " "
          << getOrDefault(props, "pw", "10n") << " "
          << getOrDefault(props, "per", "20n") << ")\n";
    } else if (type == "vsin") {
      out << "V" << stimIdx << " " << ref << " SIN("
          << getOrDefault(props, "voff", "0") << " "
          << getOrDefault(props, "vamp", "1") << " "
          << getOrDefault(props, "freq", "1e6") << ")\n";
    }
  }

  for (const auto iid : view.instanceIds()) {
    const auto* inst = view.findInstance(iid);
    if (inst == nullptr) continue;

    const auto& instName = inst->name();
    const auto xPrefix = (!instName.empty() && instName[0] == 'X') ? "" : "X";
    out << xPrefix << instName;

    const auto* masterCell = lib.findCellById(inst->masterCellId());
    if (masterCell) {
      if (const auto* symbolView = masterCell->findView(db::DbViewType::Symbol)) {
        const auto masterPinIds = symbolView->pinIds();
        for (const auto mpid : masterPinIds) {
          const auto* mpin = symbolView->findPin(mpid);
          if (!mpin) continue;
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

    for (const auto& [name, value] : inst->parameters())
      out << " " << name << "=" << value;
    out << "\n";
  }

  out << ".ends " << cell.name() << "\n";
  return out.str();
}

}  // namespace aurora::netlib
