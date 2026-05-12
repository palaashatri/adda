#pragma once

#include <string>

namespace aurora::db {
class DbCell;
class DbCellLib;
class DbView;
}

namespace aurora::netlist {

class VerilogGenerator {
 public:
  [[nodiscard]] std::string generateVerilog(const db::DbCellLib& lib, const db::DbCell& cell,
                                             const db::DbView& view) const;
};

}  // namespace aurora::netlist
