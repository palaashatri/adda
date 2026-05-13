#pragma once

#include <filesystem>
#include <string>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

// H6 — Design import wizard.
struct ImportSelection {
  enum class Format { Auto, Gds, Lef, Def, Verilog, Spice, Oasis, Cif };
  std::filesystem::path inputPath;
  Format format{Format::Auto};
  std::string targetCellName;
};

class ImportWizard {
 public:
  [[nodiscard]] static bool runAuto(db::DbCellLib& lib, const ImportSelection& sel);
};

}  // namespace aurora::layout
