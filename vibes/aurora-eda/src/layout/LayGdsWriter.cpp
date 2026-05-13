#include "layout/LayGdsWriter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbTypes.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"
#include "geom/GeomPolygon.h"

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <span>
#include <string_view>
#include <vector>

namespace aurora::layout {

// ── GDS II record type constants ──────────────────────────────────────────────
namespace rec {
  constexpr uint8_t HEADER   = 0x00;
  constexpr uint8_t BGNLIB   = 0x01;
  constexpr uint8_t LIBNAME  = 0x02;
  constexpr uint8_t UNITS    = 0x03;
  constexpr uint8_t ENDLIB   = 0x04;
  constexpr uint8_t BGNSTR   = 0x05;
  constexpr uint8_t STRNAME  = 0x06;
  constexpr uint8_t ENDSTR   = 0x07;
  constexpr uint8_t BOUNDARY = 0x08;
  constexpr uint8_t PATH     = 0x09;
  constexpr uint8_t SREF     = 0x0A;
  constexpr uint8_t TEXT     = 0x0C;
  constexpr uint8_t LAYER    = 0x0D;
  constexpr uint8_t DATATYPE = 0x0E;
  constexpr uint8_t WIDTH    = 0x0F;
  constexpr uint8_t XY       = 0x10;
  constexpr uint8_t ENDEL    = 0x11;
  constexpr uint8_t SNAME    = 0x12;
  constexpr uint8_t TEXTTYPE = 0x16;
  constexpr uint8_t STRANS   = 0x1A;
  constexpr uint8_t PATHTYPE = 0x1B;
  constexpr uint8_t ANGLE    = 0x1C;
  constexpr uint8_t STRING   = 0x19;
}

namespace dtype {
  constexpr uint8_t NONE   = 0x00;
  constexpr uint8_t INT16  = 0x02;
  constexpr uint8_t INT32  = 0x03;
  constexpr uint8_t REAL8  = 0x05;
  constexpr uint8_t ASCII  = 0x06;
}

// ── Low-level byte writers ────────────────────────────────────────────────────

static void putU8(std::ostream& o, uint8_t v) {
  o.put(static_cast<char>(v));
}

static void putU16(std::ostream& o, uint16_t v) {
  o.put(static_cast<char>(v >> 8));
  o.put(static_cast<char>(v & 0xFF));
}

static void putU32(std::ostream& o, uint32_t v) {
  o.put(static_cast<char>((v >> 24) & 0xFF));
  o.put(static_cast<char>((v >> 16) & 0xFF));
  o.put(static_cast<char>((v >>  8) & 0xFF));
  o.put(static_cast<char>( v        & 0xFF));
}

static void putU64(std::ostream& o, uint64_t v) {
  putU32(o, static_cast<uint32_t>(v >> 32));
  putU32(o, static_cast<uint32_t>(v & 0xFFFFFFFF));
}

// Convert an IEEE 754 double to IBM 64-bit hexadecimal floating point.
// IBM format: 1-bit sign | 7-bit hex exponent (biased+64) | 56-bit mantissa
static uint64_t toIbmFloat(double value) {
  if (value == 0.0) return 0;

  uint64_t sign = 0;
  if (value < 0.0) { sign = 0x8000000000000000ULL; value = -value; }

  // Normalise so that 1/16 <= |mantissa| < 1
  int exp = 0;
  while (value >= 1.0)     { value /= 16.0; ++exp; }
  while (value <  (1.0/16.0)) { value *= 16.0; --exp; }

  const uint64_t biasedExp = static_cast<uint64_t>(exp + 64);
  const uint64_t mant      = static_cast<uint64_t>(value * static_cast<double>(1ULL << 56));

  return sign | (biasedExp << 56) | (mant & 0x00FFFFFFFFFFFFFFULL);
}

// ── Record-level writers ──────────────────────────────────────────────────────

static void writeHeader(std::ostream& o, uint8_t recType, uint8_t dataType, uint16_t dataBytes) {
  putU16(o, static_cast<uint16_t>(4 + dataBytes));
  putU8(o, recType);
  putU8(o, dataType);
}

static void writeNoData(std::ostream& o, uint8_t recType) {
  writeHeader(o, recType, dtype::NONE, 0);
}

static void writeInt16s(std::ostream& o, uint8_t recType,
                        std::initializer_list<int16_t> vals) {
  writeHeader(o, recType, dtype::INT16,
              static_cast<uint16_t>(vals.size() * 2));
  for (int16_t v : vals)
    putU16(o, static_cast<uint16_t>(v));
}

static void writeInt32s(std::ostream& o, uint8_t recType,
                        std::span<const int32_t> vals) {
  writeHeader(o, recType, dtype::INT32,
              static_cast<uint16_t>(vals.size() * 4));
  for (int32_t v : vals)
    putU32(o, static_cast<uint32_t>(v));
}

static void writeString(std::ostream& o, uint8_t recType, std::string_view str) {
  const uint16_t padLen = static_cast<uint16_t>((str.size() + 1) & ~1u);  // round up to even
  writeHeader(o, recType, dtype::ASCII, padLen);
  o.write(str.data(), static_cast<std::streamsize>(str.size()));
  if (padLen > str.size()) putU8(o, 0);  // null pad to even length
}

static void writeReal8s(std::ostream& o, uint8_t recType,
                        std::initializer_list<double> vals) {
  writeHeader(o, recType, dtype::REAL8,
              static_cast<uint16_t>(vals.size() * 8));
  for (double v : vals)
    putU64(o, toIbmFloat(v));
}

// ── Timestamp helper (12 int16s: last-modified then last-access) ──────────────
static std::vector<int16_t> currentTimestamp() {
  const std::time_t now = std::time(nullptr);
  const std::tm*   tm  = std::gmtime(&now);
  std::vector<int16_t> ts(12);
  const auto fill = [&](int off) {
    if (tm) {
      ts[off + 0] = static_cast<int16_t>(tm->tm_year);
      ts[off + 1] = static_cast<int16_t>(tm->tm_mon + 1);
      ts[off + 2] = static_cast<int16_t>(tm->tm_mday);
      ts[off + 3] = static_cast<int16_t>(tm->tm_hour);
      ts[off + 4] = static_cast<int16_t>(tm->tm_min);
      ts[off + 5] = static_cast<int16_t>(tm->tm_sec);
    }
  };
  fill(0); fill(6);
  return ts;
}

static void writeBgnTs(std::ostream& o, uint8_t recType) {
  const auto ts = currentTimestamp();
  writeHeader(o, recType, dtype::INT16, 24);  // 12 × int16
  for (int16_t v : ts) putU16(o, static_cast<uint16_t>(v));
}

// ── Geometry element writers ──────────────────────────────────────────────────

static void writeBoundaryXY(std::ostream& o, std::span<const int32_t> xy) {
  // xy must already contain the closing repeat of the first vertex
  writeInt32s(o, rec::XY, xy);
}

static void writeRectElement(std::ostream& o, int gdsLayer, int gdsDatatype,
                             const geom::GeomBox& box) {
  writeNoData(o, rec::BOUNDARY);
  writeInt16s(o, rec::LAYER,    {static_cast<int16_t>(gdsLayer)});
  writeInt16s(o, rec::DATATYPE, {static_cast<int16_t>(gdsDatatype)});

  const int32_t l = static_cast<int32_t>(box.left());
  const int32_t b = static_cast<int32_t>(box.bottom());
  const int32_t r = static_cast<int32_t>(box.right());
  const int32_t t = static_cast<int32_t>(box.top());
  const int32_t xy[10] = {l, b, r, b, r, t, l, t, l, b};
  writeBoundaryXY(o, xy);
  writeNoData(o, rec::ENDEL);
}

static void writePolygonElement(std::ostream& o, int gdsLayer, int gdsDatatype,
                                const geom::GeomPolygon& poly) {
  const auto pts = poly.points();
  if (pts.empty()) return;

  writeNoData(o, rec::BOUNDARY);
  writeInt16s(o, rec::LAYER,    {static_cast<int16_t>(gdsLayer)});
  writeInt16s(o, rec::DATATYPE, {static_cast<int16_t>(gdsDatatype)});

  std::vector<int32_t> xy;
  xy.reserve((pts.size() + 1) * 2);
  for (const auto& p : pts) {
    xy.push_back(static_cast<int32_t>(p.x));
    xy.push_back(static_cast<int32_t>(p.y));
  }
  // Close polygon
  xy.push_back(static_cast<int32_t>(pts.front().x));
  xy.push_back(static_cast<int32_t>(pts.front().y));

  writeBoundaryXY(o, xy);
  writeNoData(o, rec::ENDEL);
}

static void writePathElement(std::ostream& o, int gdsLayer, int gdsDatatype,
                             const geom::GeomPath& path) {
  const auto pts = path.points();
  if (pts.empty()) return;

  writeNoData(o, rec::PATH);
  writeInt16s(o, rec::LAYER,    {static_cast<int16_t>(gdsLayer)});
  writeInt16s(o, rec::DATATYPE, {static_cast<int16_t>(gdsDatatype)});

  // PATHTYPE: 0=round, 1=miter, 2=square (bevel)
  int16_t pt = 1; // default miter
  if (path.cornerStyle() == geom::PathCornerStyle::Round)  pt = 0;
  if (path.cornerStyle() == geom::PathCornerStyle::Square) pt = 2;
  writeInt16s(o, rec::PATHTYPE, {pt});

  const int32_t w = static_cast<int32_t>(path.width());
  writeInt32s(o, rec::WIDTH, std::span<const int32_t>{&w, 1});

  std::vector<int32_t> xy;
  xy.reserve(pts.size() * 2);
  for (const auto& p : pts) {
    xy.push_back(static_cast<int32_t>(p.x));
    xy.push_back(static_cast<int32_t>(p.y));
  }
  writeBoundaryXY(o, xy);
  writeNoData(o, rec::ENDEL);
}

static void writeTextElement(std::ostream& o, int gdsLayer,
                             const geom::GeomPoint& origin,
                             const std::string& text) {
  if (text.empty()) return;

  writeNoData(o, rec::TEXT);
  writeInt16s(o, rec::LAYER,    {static_cast<int16_t>(gdsLayer)});
  writeInt16s(o, rec::TEXTTYPE, {0});
  const int32_t xy[2] = {static_cast<int32_t>(origin.x), static_cast<int32_t>(origin.y)};
  writeInt32s(o, rec::XY, xy);
  writeString(o, rec::STRING, text);
  writeNoData(o, rec::ENDEL);
}

static void writeSrefElement(std::ostream& o, const std::string& masterName,
                             const db::DbTransform& transform) {
  writeNoData(o, rec::SREF);
  writeString(o, rec::SNAME, masterName);

  // Transformation: STRANS + ANGLE
  uint16_t flags = 0;
  if (transform.mirrorX) flags |= 0x8000;
  if (flags != 0 || transform.rotationDegrees != 0) {
    writeInt16s(o, rec::STRANS, {static_cast<int16_t>(flags)});
    if (transform.rotationDegrees != 0) {
      writeReal8s(o, rec::ANGLE, {static_cast<double>(transform.rotationDegrees)});
    }
  }

  const int32_t xy[2] = {static_cast<int32_t>(transform.dx), static_cast<int32_t>(transform.dy)};
  writeInt32s(o, rec::XY, xy);
  writeNoData(o, rec::ENDEL);
}

// ── Shape dispatcher ──────────────────────────────────────────────────────────

static void writeShape(std::ostream& o, const db::DbShape& shape,
                       const db::DbCellLib& lib) {
  const db::DbLayer* layer = lib.findLayer(shape.layerId());
  if (layer == nullptr) return;
  const int gl = layer->gdsLayer();
  const int gd = layer->gdsDatatype();
  if (gl < 0) return;  // no GDS mapping assigned
  const int gdt = (gd < 0) ? 0 : gd;

  switch (shape.kind()) {
    case db::DbShapeKind::Rect:
      writeRectElement(o, gl, gdt,
                       static_cast<const db::DbRect&>(shape).box());
      break;
    case db::DbShapeKind::Polygon:
      writePolygonElement(o, gl, gdt,
                          static_cast<const db::DbPolygon&>(shape).polygon());
      break;
    case db::DbShapeKind::Path:
      writePathElement(o, gl, gdt,
                       static_cast<const db::DbPath&>(shape).path());
      break;
    case db::DbShapeKind::Text: {
      const auto& t = static_cast<const db::DbText&>(shape);
      writeTextElement(o, gl, t.origin(), t.text());
      break;
    }
  }
}

// ── Public API ────────────────────────────────────────────────────────────────

bool LayGdsWriter::write(const db::DbCellLib& lib,
                         const std::filesystem::path& path,
                         double dbuPerMicron) const {
  std::ofstream o(path, std::ios::binary | std::ios::trunc);
  if (!o) return false;

  const double userPerDbu  = 1.0 / dbuPerMicron;      // µm per dbu
  const double metersPerDbu = userPerDbu * 1e-6;       // m per dbu

  // Library header
  writeInt16s(o, rec::HEADER, {600});
  writeBgnTs(o, rec::BGNLIB);
  writeString(o, rec::LIBNAME, lib.name());
  writeReal8s(o, rec::UNITS, {userPerDbu, metersPerDbu});

  // One GDS structure per cell with a layout view
  for (const db::DbId cid : lib.cellIds()) {
    const db::DbCell* cell = lib.findCellById(cid);
    if (cell == nullptr) continue;
    const db::DbView* view = cell->findView(db::DbViewType::Layout);
    if (view == nullptr) continue;

    writeBgnTs(o, rec::BGNSTR);
    writeString(o, rec::STRNAME, cell->name());

    for (const db::DbId sid : view->shapeIds()) {
      const db::DbShape* shape = view->findShape(sid);
      if (shape != nullptr) writeShape(o, *shape, lib);
    }

    for (const db::DbId iid : view->instanceIds()) {
      const db::DbInstance* inst = view->findInstance(iid);
      if (inst == nullptr) continue;
      const db::DbCell* master = lib.findCellById(inst->masterCellId());
      if (master != nullptr) {
        writeSrefElement(o, master->name(), inst->transform());
      }
    }

    writeNoData(o, rec::ENDSTR);
  }

  writeNoData(o, rec::ENDLIB);
  return o.good();
}

}  // namespace aurora::layout
