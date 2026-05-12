#include "layout/LayDefWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbTypes.h"
#include "db/DbView.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

namespace aurora::layout {

bool LayDefWriter::write(const db::DbCellLib& lib, const std::filesystem::path& path,
                          double dbuPerMicron) const {
  std::ofstream o(path);
  if (!o) return false;

  const double micronPerDbu = 1.0 / dbuPerMicron;

  o << "VERSION 5.8 ;\n";
  o << "DIVIDERCHAR \"/\" ;\n";
  o << "BUSBITCHARS \"<>\" ;\n";
  o << "DESIGN " << lib.name() << " ;\n";
  o << "UNITS DISTANCE MICRONS " << static_cast<int>(dbuPerMicron) << " ;\n\n";

  // Collect all layout-view cells for COMPONENTS
  o << "COMPONENTS";
  int totalComps = 0;
  std::ostringstream compBody;
  for (const auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    const auto* view = cell->findView(db::DbViewType::Layout);
    if (!view) continue;

    for (const auto iid : view->instanceIds()) {
      const auto* inst = view->findInstance(iid);
      if (!inst) continue;
      const auto* master = lib.findCellById(inst->masterCellId());
      if (!master) continue;
      const double x = inst->transform().dx * micronPerDbu;
      const double y = inst->transform().dy * micronPerDbu;
      compBody << "  - " << inst->name() << " " << master->name()
               << " + PLACED ( " << x << " " << y << " ) N ;\n";
      ++totalComps;
    }
  }
  o << " " << totalComps << " ;\n" << compBody.str();
  o << "END COMPONENTS\n\n";

  // NETS: emit one net per cell with connectivity
  o << "NETS";
  int totalNets = 0;
  std::ostringstream netBody;
  for (const auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    const auto* view = cell->findView(db::DbViewType::Layout);
    if (!view) continue;

    for (const auto nid : view->netIds()) {
      const auto* net = view->findNet(nid);
      if (!net) continue;
      std::string netName = net->name();
      if (netName.empty()) continue;

      netBody << "  " << netName << " (";
      bool firstPin = true;
      for (const auto pid : net->pinIds()) {
        const auto* pin = view->findPin(pid);
        if (!pin) continue;
        if (pin->instanceId() == db::kInvalidId) {
          // Top-level port
          if (!firstPin) netBody << " ";
          netBody << " PIN " << pin->name();
          firstPin = false;
        } else {
          // Instance pin
          const auto* inst = view->findInstance(pin->instanceId());
          if (!inst) continue;
          if (!firstPin) netBody << " ";
          netBody << " ( " << inst->name() << " " << pin->name() << " )";
          firstPin = false;
        }
      }
      netBody << " ) ;\n";
      ++totalNets;
    }
  }
  o << " " << totalNets << " ;\n" << netBody.str();
  o << "END NETS\n\n";

  o << "END DESIGN\n";
  return o.good();
}

}  // namespace aurora::layout
