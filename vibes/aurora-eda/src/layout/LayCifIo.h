#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

// G14 — CIF (Caltech Intermediate Form) export and import.
class LayCifIo {
 public:
  [[nodiscard]] bool write(const db::DbCellLib& lib, const std::filesystem::path& path) const;
  [[nodiscard]] bool read(db::DbCellLib& lib, const std::filesystem::path& path) const;
};

}  // namespace aurora::layout
