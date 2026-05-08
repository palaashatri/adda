#pragma once

#include <string>

namespace aurora::db {
class DbCell;
class DbView;
class DbCellLib;
}

namespace aurora::netlist {

class NetlistGenerator {
 public:
  [[nodiscard]] std::string generateSpice(const db::DbCellLib& lib, const db::DbCell& cell,
                                            const db::DbView& view) const;
};

}  // namespace aurora::netlist
