#pragma once

#include <string>
#include <vector>

namespace aurora::db {
class DbCellLib;
class DbView;
}  // namespace aurora::db

namespace aurora::netlist {
class NetlistGenerator;
}

namespace aurora::drc_lvs {

struct LvsResult {
  bool matched{false};
  std::vector<std::string> errors;
};

// Compares a schematic view with a layout view at the net/pin level.
// MVP: checks net count and net name set; a full LVS would parse extracted nets
// from layout and compare against schematic connectivity.
class LvsChecker {
 public:
  [[nodiscard]] LvsResult compare(const db::DbView& schView, const db::DbView& layView,
                                   const db::DbCellLib& lib) const;
};

}  // namespace aurora::drc_lvs
