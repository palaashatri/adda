#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

class LayGdsWriter {
 public:
  // Write all layout-view cells in lib to a GDS II binary file.
  // dbuPerMicron: database units per micron (default 1000 → 1 dbu = 1 nm).
  // Returns false if the file cannot be opened.
  [[nodiscard]] bool write(const db::DbCellLib& lib, const std::filesystem::path& path,
                           double dbuPerMicron = 1000.0) const;
};

}  // namespace aurora::layout
