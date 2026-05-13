#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::netlist {

// G8 — Verilog structural netlist import.
class VerilogImporter {
 public:
  [[nodiscard]] bool importFile(db::DbCellLib& lib, const std::filesystem::path& path) const;
};

}  // namespace aurora::netlist
