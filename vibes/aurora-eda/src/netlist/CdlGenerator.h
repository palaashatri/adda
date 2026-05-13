#pragma once

#include <string>

namespace aurora::db {
class DbCell;
class DbCellLib;
class DbView;
}

namespace aurora::netlist {

// G9 — CDL (Circuit Description Language) netlist export.
// Enhanced SPICE with device parameters, model references, $... attributes.
class CdlGenerator {
 public:
  [[nodiscard]] std::string generateCdl(const db::DbCellLib& lib, const db::DbCell& cell,
                                         const db::DbView& view) const;
};

}  // namespace aurora::netlist
