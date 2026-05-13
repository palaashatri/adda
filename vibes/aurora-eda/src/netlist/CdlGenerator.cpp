#include "netlist/CdlGenerator.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbView.h"

#include <cctype>
#include <sstream>

namespace aurora::netlist {

std::string CdlGenerator::generateCdl(const db::DbCellLib&, const db::DbCell& cell,
                                       const db::DbView& view) const {
  std::ostringstream out;
  out << "* CDL netlist for cell " << cell.name() << "\n";
  out << ".SUBCKT " << cell.name();

  // Port pins
  for (auto pid : view.pinIds()) {
    const auto* p = view.findPin(pid);
    if (!p) continue;
    if (p->instanceId() != db::kInvalidId) continue;  // not a top port
    out << " " << p->name();
  }
  out << "\n";

  // Instances
  for (auto iid : view.instanceIds()) {
    const auto* inst = view.findInstance(iid);
    if (!inst) continue;
    // Pick prefix: device parameter "type" — m/r/c/l/x; default x
    char prefix = 'X';
    auto typeIt = inst->parameters().find("type");
    if (typeIt != inst->parameters().end() && !typeIt->second.empty()) {
      prefix = static_cast<char>(std::toupper(typeIt->second[0]));
    }
    out << prefix << inst->name();
    // Pin connections
    auto pins = view.findInstancePins(inst->id());
    for (const auto* p : pins) {
      const auto* net = view.findNet(p->netId());
      out << " " << (net ? net->name() : "?");
    }
    // Model name (master cell) and parameters
    out << " " << inst->name();
    for (const auto& [k, v] : inst->parameters()) {
      if (k == "type") continue;
      out << " " << k << "=" << v;
    }
    out << "\n";
  }

  out << ".ENDS " << cell.name() << "\n";
  return out.str();
}

}  // namespace aurora::netlist
