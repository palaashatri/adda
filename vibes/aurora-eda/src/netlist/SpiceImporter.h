#pragma once

#include <filesystem>
#include <string>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::netlist {

// Parses a SPICE netlist (.sp / .cir) and populates a DbCellLib with cells,
// instances, and nets.  Supports .subckt/.ends blocks and X-element lines.
class SpiceImporter {
 public:
  SpiceImporter() = default;

  // Returns true on success. Errors can be queried via lastError().
  bool importFile(const std::filesystem::path& path, db::DbCellLib& lib);
  bool importString(const std::string& spice, db::DbCellLib& lib);

  [[nodiscard]] const std::string& lastError() const { return lastError_; }

 private:
  bool parseLines(const std::vector<std::string>& lines, db::DbCellLib& lib);

  std::string lastError_;
};

}  // namespace aurora::netlist
