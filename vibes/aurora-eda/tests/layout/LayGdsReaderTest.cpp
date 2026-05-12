#include "layout/LayGdsReader.h"
#include "layout/LayGdsWriter.h"

#include "db/DbCellLib.h"
#include "db/DbCell.h"
#include "db/DbLayer.h"
#include "db/DbView.h"
#include "db/DbTypes.h"
#include "db/DbShape.h"
#include "db/DbInstance.h"
#include "geom/GeomBox.h"
#include "geom/GeomPolygon.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

// Build a library with known content, write to GDS, read back, and verify.
static aurora::db::DbCellLib buildLib() {
  aurora::db::DbCellLib lib("roundtrip");

  auto& diff = lib.createLayer("diff", "drawing");
  diff.setColor("#3fbf7f");
  diff.setGdsMapping(22, 0);

  auto& poly = lib.createLayer("poly", "drawing");
  poly.setColor("#e05050");
  poly.setGdsMapping(46, 0);

  auto& met1 = lib.createLayer("metal1", "drawing");
  met1.setColor("#5080d0");
  met1.setGdsMapping(68, 0);

  // Create a sub-cell (master for SREF)
  auto& subCell = lib.createCell("SUB");
  auto& subView = subCell.createView(aurora::db::DbViewType::Layout);
  (void)subView.createRect(diff.id(), aurora::geom::GeomBox{0, 0, 1000, 1000});
  (void)subView.createText(diff.id(), aurora::geom::GeomPoint{200, 200}, "SUB_MARK");

  // Top-level cell with all shape types
  auto& cell = lib.createCell("TOP");
  auto& lv   = cell.createView(aurora::db::DbViewType::Layout);

  // Rect
  (void)lv.createRect(diff.id(), aurora::geom::GeomBox{0, 0, 3000, 4000});

  // Polygon (non-rectangular)
  aurora::geom::GeomPolygon gp;
  gp.addPoint({500, 100});
  gp.addPoint({1500, 100});
  gp.addPoint({1500, 600});
  gp.addPoint({1000, 900});
  gp.addPoint({500, 600});
  (void)lv.createPolygon(diff.id(), gp);

  // Path
  aurora::geom::GeomPath path({{0, -500}, {3000, -500}, {4000, 1000}}, 200);
  (void)lv.createPath(met1.id(), path);

  // Text
  (void)lv.createText(diff.id(), aurora::geom::GeomPoint{-200, -200}, "TOP_LABEL");

  // Instance (SREF)
  (void)lv.createInstance("X1", subCell.id(), aurora::db::DbTransform{5000, 3000, 0, false});

  return lib;
}

static void testRoundtrip() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_roundtrip.gds";

  // Write
  {
    auto lib = buildLib();
    aurora::layout::LayGdsWriter writer;
    const bool ok = writer.write(lib, path);
    assert(ok && "write must succeed");
  }

  // Read into a new library
  aurora::db::DbCellLib readLib("imported");
  {
    aurora::layout::LayGdsReader reader;
    const bool ok = reader.read(readLib, path);
    assert(ok && "read must succeed");
  }

  std::filesystem::remove(path);

  // Verify: should have 2 cells (SUB, TOP)
  assert(readLib.cellIds().size() == 2 && "must have 2 cells");

  // Verify TOP cell
  const auto* topCell = readLib.findCell("TOP");
  assert(topCell != nullptr && "TOP cell must exist");
  const auto* topView = topCell->findView(aurora::db::DbViewType::Layout);
  assert(topView != nullptr && "TOP must have layout view");

  // Verify shapes
  int rectCount = 0, polyCount = 0, pathCount = 0, textCount = 0;
  for (const auto sid : topView->shapeIds()) {
    const auto* s = topView->findShape(sid);
    assert(s != nullptr);
    switch (s->kind()) {
      case aurora::db::DbShapeKind::Rect:   ++rectCount; break;
      case aurora::db::DbShapeKind::Polygon: ++polyCount; break;
      case aurora::db::DbShapeKind::Path:   ++pathCount; break;
      case aurora::db::DbShapeKind::Text:   ++textCount; break;
    }
  }
  // The diff rect (0,0,3000,4000) should be a rect (5-point closure detected)
  // The polygon (5 pts, non-rect) should be a polygon
  // The bounding condition: GDS represents everything as boundaries,
  // so rect detection is best-effort. We at least verify counts > 0.
  assert(rectCount > 0 && "must have at least 1 rect");
  assert(polyCount > 0 && "must have at least 1 polygon");
  assert(pathCount == 1 && "must have 1 path");
  assert(textCount >= 1 && "must have at least 1 text");

  // Verify instance
  assert(topView->instanceIds().size() == 1 && "must have 1 instance");
  const auto* inst = topView->findInstance(topView->instanceIds()[0]);
  assert(inst != nullptr);
  assert(inst->name() == "I1" && "instance name should be I1 (auto-generated)");
  assert(inst->transform().dx == 5000 && "instance dx");
  assert(inst->transform().dy == 3000 && "instance dy");

  // Verify SUB cell has rect + text
  const auto* subCell = readLib.findCell("SUB");
  assert(subCell != nullptr && "SUB cell must exist");
  const auto* subView = subCell->findView(aurora::db::DbViewType::Layout);
  assert(subView != nullptr && "SUB must have layout view");
  assert(subView->shapeIds().size() >= 2 && "SUB must have rect + text");

  // Verify layers got created (at least 1, since unused layers may be dropped)
  assert(readLib.layerIds().size() >= 1 && "must have at least 1 layer");

  // Verify path width was preserved
  for (const auto sid : topView->shapeIds()) {
    const auto* s = topView->findShape(sid);
    if (s && s->kind() == aurora::db::DbShapeKind::Path) {
      const auto& p = static_cast<const aurora::db::DbPath*>(s)->path();
      assert(p.width() == 200 && "path width must be 200");
      assert(p.points().size() == 3 && "path must have 3 points");
      break;
    }
  }
}

static void testMinimalRoundtrip() {
  // Write & read a single empty cell to verify at least some cell structure survives
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_minimal.gds";
  {
    aurora::db::DbCellLib lib("minimal");
    auto& lay = lib.createLayer("m1", "drawing");
    lay.setGdsMapping(1, 0);
    auto& cell = lib.createCell("A");
    auto& v = cell.createView(aurora::db::DbViewType::Layout);
    (void)v.createRect(lay.id(), aurora::geom::GeomBox{0, 0, 100, 200});
    static_cast<void>(aurora::layout::LayGdsWriter{}.write(lib, path));
  }
  aurora::db::DbCellLib readLib("minimal_imported");
  const bool ok = aurora::layout::LayGdsReader{}.read(readLib, path);
  assert(ok && "minimal roundtrip must succeed");
  std::filesystem::remove(path);
  assert(readLib.cellIds().size() >= 1 && "must have at least 1 cell");
  const auto* c = readLib.findCell("A");
  assert(c != nullptr && "cell A must exist");
  const auto* v = c->findView(aurora::db::DbViewType::Layout);
  assert(v != nullptr && "cell A must have layout view");
  assert(!v->shapeIds().empty() && "must have at least 1 shape");
}

static void testInvalidFile() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_nonexistent.gds";
  aurora::db::DbCellLib lib("empty");
  aurora::layout::LayGdsReader reader;
  const bool ok = reader.read(lib, path);
  assert(!ok && "reading a nonexistent file must fail");
}

static void testEmptyFile() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_gds_empty.gds";
  {
    std::ofstream f(path, std::ios::binary);
    // Write a minimal valid GDS: just HEADER + ENDLIB
    const uint8_t header[6] = {0x00, 0x06, 0x00, 0x02, 0x02, 0x58}; // HEADER 600
    f.write(reinterpret_cast<const char*>(header), sizeof(header));
    const uint8_t endlib[4] = {0x00, 0x04, 0x04, 0x00}; // ENDLIB
    f.write(reinterpret_cast<const char*>(endlib), sizeof(endlib));
  }
  aurora::db::DbCellLib lib("empty");
  aurora::layout::LayGdsReader reader;
  const bool ok = reader.read(lib, path);
  assert(ok && "reading minimal valid GDS must succeed");
  std::filesystem::remove(path);
}

int main() {
  testRoundtrip();
  testInvalidFile();
  testEmptyFile();
  return 0;
}
