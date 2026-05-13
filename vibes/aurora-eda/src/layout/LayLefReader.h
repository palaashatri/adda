#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

// G4 — LEF import.
class LayLefReader {
 public:
  [[nodiscard]] bool read(db::DbCellLib& lib, const std::filesystem::path& path) const;
};

}  // namespace aurora::layout
