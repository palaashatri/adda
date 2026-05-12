#include "layout/LayGdsReader.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbTypes.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"
#include "geom/GeomPolygon.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace aurora::layout {

namespace {
// GDS record type constants (mirrors LayGdsWriter.cpp)
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
constexpr uint8_t ANGLE    = 0x1C;
constexpr uint8_t STRING   = 0x19;
}

struct GdsRecord {
  uint16_t totalLen;
  uint8_t  recType;
  uint8_t  dataType;
  uint16_t dataLen;
  size_t   dataOff;
};

static uint16_t readU16(const uint8_t* buf, size_t off) {
  return static_cast<uint16_t>((buf[off] << 8) | buf[off + 1]);
}

static int16_t readS16(const uint8_t* buf, size_t off) {
  return static_cast<int16_t>(readU16(buf, off));
}

static int32_t readS32(const uint8_t* buf, size_t off) {
  return static_cast<int32_t>(
      (static_cast<uint32_t>(buf[off])     << 24) |
      (static_cast<uint32_t>(buf[off + 1]) << 16) |
      (static_cast<uint32_t>(buf[off + 2]) <<  8) |
       static_cast<uint32_t>(buf[off + 3]));
}

// Parse IBM 64-bit hexadecimal float → double
static double readIbmFloat64(const uint8_t* buf, size_t off) {
  const uint64_t raw =
      (static_cast<uint64_t>(buf[off])     << 56) |
      (static_cast<uint64_t>(buf[off + 1]) << 48) |
      (static_cast<uint64_t>(buf[off + 2]) << 40) |
      (static_cast<uint64_t>(buf[off + 3]) << 32) |
      (static_cast<uint64_t>(buf[off + 4]) << 24) |
      (static_cast<uint64_t>(buf[off + 5]) << 16) |
      (static_cast<uint64_t>(buf[off + 6]) <<  8) |
       static_cast<uint64_t>(buf[off + 7]);
  if (raw == 0) return 0.0;
  const double sign = (raw & 0x8000000000000000ULL) ? -1.0 : 1.0;
  const int32_t exp = static_cast<int32_t>((raw >> 56) & 0x7F) - 64;
  const double mant = static_cast<double>(raw & 0x00FFFFFFFFFFFFFFULL) / (1ULL << 56);
  return sign * mant * std::pow(16.0, exp);
}

static std::vector<GdsRecord> parseRecords(const uint8_t* buf, size_t size) {
  std::vector<GdsRecord> recs;
  for (size_t i = 0; i + 3 < size; ) {
    const uint16_t len = readU16(buf, i);
    if (len < 4) break;
    recs.push_back({len, buf[i + 2], buf[i + 3], static_cast<uint16_t>(len - 4), i + 4});
    i += len;
  }
  return recs;
}

// Ensure a layer exists for a given GDS layer/datatype pair.
// Returns the DbLayer id, or kInvalidId if layer number is negative.
static db::DbId resolveGdsLayer(db::DbCellLib& lib, int16_t gdsLayer, int16_t gdsDatatype) {
  if (gdsLayer < 0) return db::kInvalidId;
  // Scan existing layers for a matching GDS mapping
  for (const auto lid : lib.layerIds()) {
    const auto* l = lib.findLayer(lid);
    if (l && l->gdsLayer() == gdsLayer && l->gdsDatatype() == gdsDatatype)
      return lid;
  }
  // Create a new layer
  const std::string layerName = "gds_" + std::to_string(gdsLayer) + "_" + std::to_string(gdsDatatype);
  auto& layer = lib.createLayer(layerName, "drawing");
  layer.setGdsMapping(gdsLayer, gdsDatatype);
  // Assign distinct colors so layers are visible
  static const char* palette[] = {
    "#3fbf7f", "#e05050", "#5080d0", "#d9b34c", "#c080e0",
    "#40c0c0", "#e08040", "#80a0e0", "#c0c040", "#e060a0",
  };
  static int palIdx = 0;
  layer.setColor(palette[palIdx++ % 10]);
  return layer.id();
}

bool LayGdsReader::read(db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ifstream f(path, std::ios::binary | std::ios::ate);
  if (!f) return false;
  const auto fileSize = static_cast<size_t>(f.tellg());
  if (fileSize < 4) return false;
  f.seekg(0);
  std::vector<uint8_t> buf(fileSize);
  if (!f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(fileSize)))
    return false;

  const auto recs = parseRecords(buf.data(), fileSize);

  // First pass: collect cell names and build a mapping.
  // GDS structures appear before any references to them, so we create cells
  // on-the-fly during the second pass.
  std::map<std::string, db::DbCell*> cellsByName;
  db::DbCell* currentCell = nullptr;
  db::DbView* currentView = nullptr;

  // Collect (layer, datatype) pairs so we know how many layers to create
  // We'll create layers on demand.

  double userUnitsPerDbu = 1e-3; // default: 1 DBU = 0.001 user units = 1 nm at 1000 DBU/µm
  double metersPerDbu = 1e-9;    // default

  size_t i = 0;
  while (i < recs.size()) {
    const auto& rec = recs[i];
    switch (rec.recType) {
      case UNITS: {
        if (rec.dataLen >= 16) {
          userUnitsPerDbu = readIbmFloat64(buf.data(), rec.dataOff);
          metersPerDbu    = readIbmFloat64(buf.data(), rec.dataOff + 8);
        }
        break;
      }
      case BGNSTR:
        currentCell = nullptr;
        currentView = nullptr;
        break;
      case STRNAME: {
        const std::string name(reinterpret_cast<const char*>(buf.data() + rec.dataOff),
                                rec.dataLen);
        // Strip null padding
        std::string clean = name;
        clean.erase(std::find(clean.begin(), clean.end(), '\0'), clean.end());
        auto it = cellsByName.find(clean);
        if (it != cellsByName.end()) {
          currentCell = it->second;
        } else {
          auto& cell = lib.createCell(clean);
          currentCell = &cell;
          cellsByName[clean] = &cell;
        }
        currentView = &currentCell->createView(db::DbViewType::Layout);
        break;
      }
      case ENDSTR:
        currentCell = nullptr;
        currentView = nullptr;
        break;
      case BOUNDARY:
      case PATH:
      case TEXT:
      case SREF:
        // Handled below - these require data from subsequent records
        break;
      default:
        break;
    }
    ++i;
  }

  // Reset for second pass: parse elements within each structure
  currentCell = nullptr;
  currentView = nullptr;

  // Element state (persists across multiple records within an element)
  int16_t curLayer  = 0;
  int16_t curDatatype = 0;
  int32_t curWidth  = 0;
  std::vector<int32_t> curXY;
  std::string curString;
  uint16_t curStrans  = 0;
  double   curAngle   = 0.0;
  bool     inElement  = false;

  enum class ElKind { None, Boundary, Path, Text, Sref };
  ElKind curKind = ElKind::None;

  for (size_t ri = 0; ri < recs.size(); ++ri) {
    const auto& rec = recs[ri];

    switch (rec.recType) {
      case BGNSTR:
        currentCell = nullptr;
        currentView = nullptr;
        inElement = false;
        curKind = ElKind::None;
        break;

      case STRNAME: {
        const std::string name(reinterpret_cast<const char*>(buf.data() + rec.dataOff), rec.dataLen);
        std::string clean = name;
        clean.erase(std::find(clean.begin(), clean.end(), '\0'), clean.end());
        auto it = cellsByName.find(clean);
        if (it != cellsByName.end()) currentCell = it->second;
        if (currentCell) currentView = currentCell->findView(db::DbViewType::Layout);
        break;
      }

      case ENDSTR:
        currentCell = nullptr;
        currentView = nullptr;
        break;

      case BOUNDARY:
        curKind = ElKind::Boundary;
        inElement = true;
        curLayer = 0;
        curDatatype = 0;
        curXY.clear();
        break;

      case PATH:
        curKind = ElKind::Path;
        inElement = true;
        curLayer = 0;
        curDatatype = 0;
        curWidth = 0;
        curXY.clear();
        break;

      case TEXT:
        curKind = ElKind::Text;
        inElement = true;
        curLayer = 0;
        curDatatype = 0;
        curXY.clear();
        curString.clear();
        break;

      case SREF:
        curKind = ElKind::Sref;
        inElement = true;
        curStrans = 0;
        curAngle = 0.0;
        curXY.clear();
        curString.clear();
        break;

      case LAYER:
        if (rec.dataLen >= 2) curLayer = readS16(buf.data(), rec.dataOff);
        break;

      case DATATYPE:
        if (rec.dataLen >= 2) curDatatype = readS16(buf.data(), rec.dataOff);
        break;

      case WIDTH:
        if (rec.dataLen >= 4) curWidth = readS32(buf.data(), rec.dataOff);
        break;

      case XY: {
        const int count = rec.dataLen / 4;
        curXY.clear();
        curXY.reserve(count);
        for (int p = 0; p < count; ++p)
          curXY.push_back(readS32(buf.data(), rec.dataOff + p * 4));
        break;
      }

      case STRING:
        if (rec.dataLen > 0) {
          const std::string s(reinterpret_cast<const char*>(buf.data() + rec.dataOff),
                               rec.dataLen);
          curString = s;
          curString.erase(std::find(curString.begin(), curString.end(), '\0'), curString.end());
        }
        break;

      case SNAME:
        if (rec.dataLen > 0) {
          const std::string s(reinterpret_cast<const char*>(buf.data() + rec.dataOff),
                               rec.dataLen);
          curString = s;
          curString.erase(std::find(curString.begin(), curString.end(), '\0'), curString.end());
        }
        break;

      case STRANS:
        if (rec.dataLen >= 2) curStrans = readU16(buf.data(), rec.dataOff);
        break;

      case ANGLE:
        if (rec.dataLen >= 8) curAngle = readIbmFloat64(buf.data(), rec.dataOff);
        break;

      case ENDEL: {
        if (!inElement || !currentView) {
          inElement = false;
          curKind = ElKind::None;
          break;
        }
        const auto dblId = resolveGdsLayer(lib, curLayer, curDatatype);
        if (dblId == db::kInvalidId) {
          inElement = false;
          curKind = ElKind::None;
          break;
        }
        switch (curKind) {
          case ElKind::Boundary: {
            if (curXY.size() >= 8) {
              const int numPts = curXY.size() / 2;
              // Check if this is a 5-point closed rectangle
              bool isRect = false;
              int32_t rMinX = 0, rMaxX = 0, rMinY = 0, rMaxY = 0;
              if (numPts == 5 && curXY[0] == curXY[8] && curXY[1] == curXY[9]) {
                int32_t xs[4] = {curXY[0], curXY[2], curXY[4], curXY[6]};
                int32_t ys[4] = {curXY[1], curXY[3], curXY[5], curXY[7]};
                rMinX = xs[0]; rMaxX = xs[0]; rMinY = ys[0]; rMaxY = ys[0];
                for (int k = 1; k < 4; ++k) {
                  if (xs[k] < rMinX) rMinX = xs[k];
                  if (xs[k] > rMaxX) rMaxX = xs[k];
                  if (ys[k] < rMinY) rMinY = ys[k];
                  if (ys[k] > rMaxY) rMaxY = ys[k];
                }
                if (rMinX < rMaxX && rMinY < rMaxY) {
                  isRect = true;
                  for (int k = 0; k < 4 && isRect; ++k) {
                    bool corner = (xs[k] == rMinX || xs[k] == rMaxX) &&
                                  (ys[k] == rMinY || ys[k] == rMaxY);
                    if (!corner) isRect = false;
                  }
                }
              }
              if (isRect) {
                (void)currentView->createRect(dblId,
                    geom::GeomBox{rMinX, rMinY, rMaxX, rMaxY});
              } else {
                geom::GeomPolygon poly;
                for (int p = 0; p < numPts - 1; ++p)
                  poly.addPoint(geom::GeomPoint{curXY[p * 2], curXY[p * 2 + 1]});
                (void)currentView->createPolygon(dblId, poly);
              }
            }
            break;
          }
          case ElKind::Path: {
            if (curXY.size() >= 4) {
              std::vector<geom::GeomPoint> pts;
              const int numPts = curXY.size() / 2;
              pts.reserve(numPts);
              for (int p = 0; p < numPts; ++p)
                pts.emplace_back(curXY[p * 2], curXY[p * 2 + 1]);
              (void)currentView->createPath(dblId, geom::GeomPath{pts, static_cast<geom::DbUnit>(curWidth)});
            }
            break;
          }
          case ElKind::Text: {
            if (curXY.size() >= 2) {
              (void)currentView->createText(dblId,
                  geom::GeomPoint{curXY[0], curXY[1]}, curString);
            }
            break;
          }
          case ElKind::Sref: {
            // Look up the master cell
            const auto* masterCell = lib.findCell(curString);
            if (masterCell && curXY.size() >= 2) {
              db::DbTransform xform;
              xform.dx = curXY[0];
              xform.dy = curXY[1];
              xform.mirrorX = (curStrans & 0x8000) != 0;
              xform.rotationDegrees = curAngle;
              // Generate a unique instance name
              static uint64_t instCounter = 1;
              const std::string instName = "I" + std::to_string(instCounter++);
              (void)currentView->createInstance(instName, masterCell->id(), xform);
            }
            break;
          }
          default: break;
        }
        inElement = false;
        curKind = ElKind::None;
        break;
      }

      default:
        break;
    }
  }

  return true;
}

}  // namespace aurora::layout
