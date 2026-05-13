#include "layout/LayOasisReader.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <cstdint>
#include <fstream>
#include <map>

namespace aurora::layout {

namespace {

std::uint64_t readUnsignedInt(std::istream& in) {
  std::uint64_t v = 0;
  unsigned shift = 0;
  while (in) {
    int c = in.get();
    if (c < 0) break;
    auto b = static_cast<std::uint8_t>(c);
    v |= static_cast<std::uint64_t>(b & 0x7F) << shift;
    if (!(b & 0x80)) break;
    shift += 7;
    if (shift >= 64) break;
  }
  return v;
}

std::string readString(std::istream& in, std::uint64_t len) {
  std::string s;
  s.resize(len);
  in.read(s.data(), static_cast<std::streamsize>(len));
  return s;
}

db::DbId ensureLayer(db::DbCellLib& lib, int n, std::map<int, db::DbId>& cache) {
  auto it = cache.find(n);
  if (it != cache.end()) return it->second;
  auto& l = lib.createLayer("oasis_L" + std::to_string(n), "drawing");
  cache[n] = l.id();
  return l.id();
}

}  // namespace

bool LayOasisReader::read(db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;

  char magic[14];
  in.read(magic, 13);
  magic[13] = 0;
  if (std::string(magic, 13).find("%SEMI-OASIS") == std::string::npos) {
    in.seekg(0);
  }

  std::map<int, db::DbId> layerCache;
  db::DbCell* currentCell = nullptr;
  db::DbView* currentView = nullptr;

  while (in) {
    int rec = in.get();
    if (rec < 0) break;
    if (rec == 1) {
      auto vlen = readUnsignedInt(in);
      (void)readString(in, vlen);
      (void)readUnsignedInt(in);  // unit
      (void)readUnsignedInt(in);  // offset flag
    } else if (rec == 2) {
      break;
    } else if (rec == 13) {
      auto nlen = readUnsignedInt(in);
      std::string name = readString(in, nlen);
      auto* cell = lib.findCell(name);
      if (!cell) cell = &lib.createCell(name);
      currentCell = cell;
      currentView = &cell->createView(db::DbViewType::Layout);
    } else if (rec == 20) {
      auto layerNum = static_cast<int>(readUnsignedInt(in));
      (void)readUnsignedInt(in);  // datatype
      auto x = static_cast<long long>(readUnsignedInt(in));
      auto y = static_cast<long long>(readUnsignedInt(in));
      auto w = static_cast<long long>(readUnsignedInt(in));
      auto h = static_cast<long long>(readUnsignedInt(in));
      if (currentView) {
        auto lid = ensureLayer(lib, layerNum, layerCache);
        (void)currentView->createRect(lid, geom::GeomBox{x, y, x + w, y + h});
      }
    } else {
      break;
    }
  }
  (void)currentCell;
  return true;
}

}  // namespace aurora::layout
