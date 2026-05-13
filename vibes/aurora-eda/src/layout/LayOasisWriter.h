#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

// G12 — OASIS export (a more compact GDS alternative).
// Minimal v1.0 START/END framing with CELL/RECTANGLE records.
class LayOasisWriter {
 public:
  [[nodiscard]] bool write(const db::DbCellLib& lib, const std::filesystem::path& path) const;
};

}  // namespace aurora::layout
