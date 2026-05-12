#pragma once

#include <filesystem>

namespace aurora::db {
class DbCellLib;
}

namespace aurora::layout {

class LayDefWriter {
 public:
  [[nodiscard]] bool write(const db::DbCellLib& lib, const std::filesystem::path& path,
                           double dbuPerMicron = 1000.0) const;
};

}  // namespace aurora::layout
