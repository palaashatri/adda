#include "layout/LayGdsWriter.h"

#include "db/DbCellLib.h"
#include "db/DbCell.h"
#include "db/DbLayer.h"
#include "db/DbView.h"
#include "db/DbTypes.h"
#include "geom/GeomBox.h"
#include "geom/GeomPolygon.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <vector>

// Read the full file into a byte buffer.
static std::vector<uint8_t> readFile(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  assert(f && "file must be readable");
  return {std::istreambuf_iterator<char>(f), {}};
}

// Pull a big-endian uint16 from the buffer.
static uint16_t u16At(const std::vector<uint8_t>& buf, size_t off) {
  return static_cast<uint16_t>((buf[off] << 8) | buf[off + 1]);
}

// Pull a big-endian int32 from the buffer.
static int32_t i32At(const std::vector<uint8_t>& buf, size_t off) {
  return static_cast<int32_t>(
      (static_cast<uint32_t>(buf[off])     << 24) |
      (static_cast<uint32_t>(buf[off + 1]) << 16) |
      (static_cast<uint32_t>(buf[off + 2]) <<  8) |
       static_cast<uint32_t>(buf[off + 3]));
}

// Locate the byte offset of the first occurrence of a record type.
static size_t findRecord(const std::vector<uint8_t>& buf, uint8_t recType) {
  for (size_t i = 0; i + 3 < buf.size(); ) {
    if (buf[i + 2] == recType) return i;
    const size_t len = u16At(buf, i);
    assert(len >= 4 && "malformed GDS record length");
    i += len;
  }
  return SIZE_MAX;
}

// Collect all record-type offsets for structured traversal.
struct Record { size_t offset; uint8_t type; uint8_t dtype; uint16_t dataLen; };
static std::vector<Record> parseRecords(const std::vector<uint8_t>& buf) {
  std::vector<Record> recs;
  for (size_t i = 0; i + 3 < buf.size(); ) {
    const uint16_t len = u16At(buf, i);
    assert(len >= 4);
    recs.push_back({i, buf[i + 2], buf[i + 3], static_cast<uint16_t>(len - 4)});
    i += len;
  }
  return recs;
}

static aurora::db::DbCellLib buildLib() {
  aurora::db::DbCellLib lib("testlib");

  auto& diffLayer = lib.createLayer("diff", "drawing");
  diffLayer.setColor("#3fbf7f");
  diffLayer.setGdsMapping(22, 0);

  auto& polyLayer = lib.createLayer("poly", "drawing");
  polyLayer.setColor("#e05050");
  polyLayer.setGdsMapping(46, 0);

  auto& met1Layer = lib.createLayer("metal1", "drawing");
  met1Layer.setColor("#5080d0");
  met1Layer.setGdsMapping(68, 0);

  auto& cell = lib.createCell("NMOS");
  auto& lv   = cell.createView(aurora::db::DbViewType::Layout);

  using B = aurora::geom::GeomBox;
  using P = aurora::geom::GeomPoint;

  (void)lv.createRect(diffLayer.id(), B{0, 0, 3000, 4000});
  (void)lv.createRect(polyLayer.id(), B{1200, -400, 1800, 4400});
  (void)lv.createRect(met1Layer.id(), B{200, 3200, 2800, 4000});

  aurora::geom::GeomPolygon poly;
  poly.addPoint(P{500, 100});
  poly.addPoint(P{1000, 100});
  poly.addPoint(P{1000, 600});
  poly.addPoint(P{500, 600});
  (void)lv.createPolygon(diffLayer.id(), poly);

  aurora::geom::GeomPath path({P{0, -500}, P{3000, -500}}, 200);
  (void)lv.createPath(met1Layer.id(), path);

  (void)lv.createText(diffLayer.id(), P{-200, -200}, "NMOS");

  return lib;
}

// ── Tests ────────────────────────────────────────────────────────────────────

static void testFileIsCreated() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_test.gds";
  auto lib = buildLib();
  aurora::layout::LayGdsWriter writer;
  const bool ok = writer.write(lib, path);
  assert(ok && "write must return true");
  assert(std::filesystem::exists(path) && "output file must exist");
  assert(std::filesystem::file_size(path) > 0 && "output file must be non-empty");
  std::filesystem::remove(path);
}

static void testMagicHeaderAndVersion() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_hdr.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf = readFile(path);
  std::filesystem::remove(path);

  // First record: HEADER (type 0x00), data type INT16 (0x02), length 6, value 600
  assert(buf.size() >= 6);
  assert(u16At(buf, 0) == 6    && "HEADER record length = 6");
  assert(buf[2]        == 0x00 && "record type = HEADER");
  assert(buf[3]        == 0x02 && "data type = INT16");
  assert(u16At(buf, 4) == 600  && "GDS II version = 600");
}

static void testLibnameRecord() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_name.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  // Find LIBNAME record (0x02)
  size_t idx = SIZE_MAX;
  for (size_t i = 0; i < recs.size(); ++i) {
    if (recs[i].type == 0x02) { idx = i; break; }
  }
  assert(idx != SIZE_MAX && "LIBNAME record must exist");
  // Data bytes start at recs[idx].offset + 4
  const size_t dataOff = recs[idx].offset + 4;
  const std::string name(reinterpret_cast<const char*>(buf.data() + dataOff),
                         recs[idx].dataLen);
  // Name is "testlib" (possibly padded with a null byte)
  assert(name.find("testlib") == 0 && "LIBNAME must start with 'testlib'");
}

static void testEndlibPresent() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_end.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  // ENDLIB (0x04) must be the very last record, zero data bytes
  assert(!recs.empty());
  assert(recs.back().type    == 0x04 && "last record must be ENDLIB");
  assert(recs.back().dataLen == 0    && "ENDLIB has no data");
}

static void testBoundaryRecordsForRects() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_rects.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  // Count BOUNDARY records (0x08) — we have 4 (3 rects + 1 polygon)
  int boundaries = 0;
  for (const auto& r : recs)
    if (r.type == 0x08) ++boundaries;
  assert(boundaries == 4 && "4 BOUNDARY records expected (3 rects + 1 polygon)");
}

static void testPathRecord() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_path.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  int paths = 0;
  for (const auto& r : recs)
    if (r.type == 0x09) ++paths;
  assert(paths == 1 && "1 PATH record expected");
}

static void testTextRecord() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_text.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  int texts = 0;
  for (const auto& r : recs)
    if (r.type == 0x0C) ++texts;
  assert(texts == 1 && "1 TEXT element record expected");
}

static void testRectCoordinates() {
  // Verify the first rect (diff, 0,0,3000,4000) produces correct XY data.
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_coord.gds";
  auto lib = buildLib();
  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  // Find the first XY record following the first BOUNDARY
  bool inBoundary = false;
  for (size_t i = 0; i < recs.size(); ++i) {
    if (recs[i].type == 0x08) { inBoundary = true; }
    if (inBoundary && recs[i].type == 0x10) {  // XY
      // 5 coordinate pairs = 10 int32 values = 40 data bytes
      assert(recs[i].dataLen == 40 && "rect XY must have 5 pairs (40 bytes)");
      const size_t dataOff = recs[i].offset + 4;
      // First pair should be (left, bottom) = (0, 0)
      assert(i32At(buf, dataOff +  0) == 0    && "rect x0 = 0");
      assert(i32At(buf, dataOff +  4) == 0    && "rect y0 = 0");
      // Third pair (top-right) = (3000, 4000)
      assert(i32At(buf, dataOff + 16) == 3000 && "rect x2 = 3000");
      assert(i32At(buf, dataOff + 20) == 4000 && "rect y2 = 4000");
      // Fifth pair (closing) = first pair
      assert(i32At(buf, dataOff + 32) == 0 && "closing x = 0");
      assert(i32At(buf, dataOff + 36) == 0 && "closing y = 0");
      break;
    }
  }
}

static void testSkipsLayerWithoutGdsMapping() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_nomapping.gds";
  aurora::db::DbCellLib lib("unmapped");
  auto& layer = lib.createLayer("no_gds", "drawing");
  (void)layer;  // no setGdsMapping call → gdsLayer() = -1

  auto& cell = lib.createCell("TestCell");
  auto& lv   = cell.createView(aurora::db::DbViewType::Layout);
  (void)lv.createRect(layer.id(), aurora::geom::GeomBox{0, 0, 1000, 1000});

  const bool ok = aurora::layout::LayGdsWriter{}.write(lib, path);
  assert(ok);
  const auto buf  = readFile(path);
  const auto recs = parseRecords(buf);
  std::filesystem::remove(path);

  // No BOUNDARY records should appear since the layer has no GDS mapping
  for (const auto& r : recs)
    assert(r.type != 0x08 && "unmapped layer shapes must not emit BOUNDARY");
}

int main() {
  testFileIsCreated();
  testMagicHeaderAndVersion();
  testLibnameRecord();
  testEndlibPresent();
  testBoundaryRecordsForRects();
  testPathRecord();
  testTextRecord();
  testRectCoordinates();
  testSkipsLayerWithoutGdsMapping();
  return 0;
}
