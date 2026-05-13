#pragma once

#include <filesystem>

namespace aurora::db {
class DbView;
class DbCellLib;
}

namespace aurora::layout {

// G16 — PDF/PNG/SVG export (document-quality vector/raster).
class LayImageExport {
 public:
  // Write SVG of the layout view (vector, viewable anywhere).
  [[nodiscard]] bool writeSvg(const db::DbCellLib& lib, const db::DbView& view,
                              const std::filesystem::path& path) const;

  // Write a minimal PDF (single page) of the layout view.
  [[nodiscard]] bool writePdf(const db::DbCellLib& lib, const db::DbView& view,
                              const std::filesystem::path& path) const;

  // Write PPM (raster) of the layout view. PNG would need zlib; PPM is portable.
  [[nodiscard]] bool writePpm(const db::DbCellLib& lib, const db::DbView& view,
                              const std::filesystem::path& path, int widthPx = 800) const;
};

}  // namespace aurora::layout
