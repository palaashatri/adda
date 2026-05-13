#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

// G13 — OASIS import (matches LayOasisWriter format).
class LayOasisReader {
 public:
  [[nodiscard]] bool read(db::DbCellLib& lib, const std::filesystem::path& path) const;
};

}  // namespace aurora::layout
