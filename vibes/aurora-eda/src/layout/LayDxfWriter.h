#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

// G15 — DXF export (AutoCAD exchange).
class LayDxfWriter {
 public:
  [[nodiscard]] bool write(const db::DbCellLib& lib, const std::filesystem::path& path) const;
};

}  // namespace aurora::layout
