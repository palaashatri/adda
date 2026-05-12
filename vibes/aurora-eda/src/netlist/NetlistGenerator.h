#pragma once

#include <string>

namespace aurora::db {
class DbCell;
class DbCellLib;
class DbView;
}

namespace aurora::netlist {

class NetlistGenerator {
 public:
  [[nodiscard]] std::string generateSpice(const db::DbCellLib& lib, const db::DbCell& cell,
                                           const db::DbView& view) const;
  // Multi-sheet netlist: merge all schematic-view cells, connecting nets with same name
  [[nodiscard]] std::string generateSpiceMulti(const db::DbCellLib& lib) const;
};

}  // namespace aurora::netlist
