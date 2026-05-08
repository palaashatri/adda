#include "drc_lvs/LvsChecker.h"

#include "db/DbCellLib.h"
#include "db/DbNet.h"
#include "db/DbView.h"

#include <set>
#include <sstream>

namespace aurora::drc_lvs {

LvsResult LvsChecker::compare(const db::DbView& schView, const db::DbView& layView,
                                const db::DbCellLib& lib) const {
  LvsResult result;

  // Collect net names from each view
  auto collectNets = [](const db::DbView& view) -> std::set<std::string> {
    std::set<std::string> names;
    for (const auto netId : view.netIds()) {
      const auto* net = view.findNet(netId);
      if (net) names.insert(net->name());
    }
    return names;
  };

  const auto schNets = collectNets(schView);
  const auto layNets = collectNets(layView);

  // Net count comparison
  if (schNets.size() != layNets.size()) {
    std::ostringstream msg;
    msg << "Net count mismatch: schematic has " << schNets.size()
        << ", layout has " << layNets.size();
    result.errors.push_back(msg.str());
  }

  // Nets in schematic but not layout
  for (const auto& name : schNets) {
    if (layNets.find(name) == layNets.end()) {
      result.errors.push_back("Net in schematic but not layout: " + name);
    }
  }

  // Nets in layout but not schematic
  for (const auto& name : layNets) {
    if (schNets.find(name) == schNets.end()) {
      result.errors.push_back("Net in layout but not schematic: " + name);
    }
  }

  // Pin count per net
  auto pinCount = [](const db::DbView& view, const std::string& netName) -> std::size_t {
    for (const auto netId : view.netIds()) {
      const auto* net = view.findNet(netId);
      if (net && net->name() == netName) return net->pinIds().size();
    }
    return 0;
  };

  for (const auto& name : schNets) {
    if (layNets.find(name) == layNets.end()) continue;
    const auto sc = pinCount(schView, name);
    const auto lc = pinCount(layView, name);
    if (sc != lc) {
      std::ostringstream msg;
      msg << "Pin count mismatch on net \"" << name << "\": schematic=" << sc
          << ", layout=" << lc;
      result.errors.push_back(msg.str());
    }
  }

  result.matched = result.errors.empty();
  return result;
}

}  // namespace aurora::drc_lvs
