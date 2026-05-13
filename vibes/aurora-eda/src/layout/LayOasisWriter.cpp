#include "layout/LayOasisWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <cstdint>
#include <fstream>

namespace aurora::layout {

namespace {

void writeUnsignedInt(std::ostream& o, std::uint64_t v) {
  while (v >= 0x80) {
    auto b = static_cast<std::uint8_t>((v & 0x7F) | 0x80);
    o.put(static_cast<char>(b));
    v >>= 7;
  }
  o.put(static_cast<char>(v));
}

void writeRecord(std::ostream& o, std::uint8_t id) { o.put(static_cast<char>(id)); }

}  // namespace

bool LayOasisWriter::write(const db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;

  // Magic bytes
  const char magic[] = "%SEMI-OASIS\r\n";
  out.write(magic, sizeof(magic) - 1);

  // START record (id=1)
  writeRecord(out, 1);
  // version
  const char ver[] = "1.0";
  writeUnsignedInt(out, sizeof(ver) - 1);
  out.write(ver, sizeof(ver) - 1);
  writeUnsignedInt(out, 1000);  // unit
  writeUnsignedInt(out, 0);     // offset-flag = 0 (offsets at end)

  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    const auto* view = cell->findView(db::DbViewType::Layout);
    if (!view) continue;

    // CELL record (id=13)
    writeRecord(out, 13);
    const auto& name = cell->name();
    writeUnsignedInt(out, name.size());
    out.write(name.data(), name.size());

    for (auto sid : view->shapeIds()) {
      const auto* s = view->findShape(sid);
      if (!s || s->kind() != db::DbShapeKind::Rect) continue;
      const auto& b = static_cast<const db::DbRect*>(s)->box();
      // RECTANGLE record (id=20)
      writeRecord(out, 20);
      writeUnsignedInt(out, static_cast<std::uint64_t>(s->layerId()));
      writeUnsignedInt(out, 0);  // datatype
      writeUnsignedInt(out, static_cast<std::uint64_t>(std::max<long long>(0, b.left())));
      writeUnsignedInt(out, static_cast<std::uint64_t>(std::max<long long>(0, b.bottom())));
      writeUnsignedInt(out, static_cast<std::uint64_t>(std::max<long long>(0, b.width())));
      writeUnsignedInt(out, static_cast<std::uint64_t>(std::max<long long>(0, b.height())));
    }
  }

  // END record (id=2)
  writeRecord(out, 2);
  return static_cast<bool>(out);
}

}  // namespace aurora::layout
