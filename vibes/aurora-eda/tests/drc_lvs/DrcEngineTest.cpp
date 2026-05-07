#include "drc_lvs/DrcEngine.h"
#include "drc_lvs/LvsChecker.h"
#include "db/DbCellLib.h"
#include "db/DbCell.h"
#include "db/DbView.h"
#include "tech/TechDatabase.h"
#include "geom/GeomBox.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <string>

static std::filesystem::path makeTempDir() {
  const auto base = std::filesystem::temp_directory_path() / "aurora_drc_test";
  std::filesystem::create_directories(base);
  return base;
}

static void writeTechJson(const std::filesystem::path& path) {
  std::ofstream f(path);
  f << R"({
  "name": "testpdk",
  "units": {"database_unit": "nm", "dbu_per_micron": 1000},
  "layers": [
    {"id": 1, "name": "metal1", "purpose": "drawing",
     "color": "#5080d0", "gds_layer": 31, "gds_datatype": 0,
     "default_width": 200, "default_spacing": 200}
  ],
  "rules": []
})";
}

static int failures = 0;
#define CHECK(expr) do { \
  if (!(expr)) { \
    std::fprintf(stderr, "FAIL: %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
    ++failures; \
  } \
} while(0)

static void test_no_violations_compliant() {
  const auto dir = makeTempDir();
  const auto techPath = dir / "tech_compliant.json";
  writeTechJson(techPath);

  aurora::tech::TechDatabase tech;
  CHECK(tech.loadFromJsonFile(techPath));

  aurora::db::DbCellLib lib("testlib");
  auto& layer = lib.createLayer("metal1", "drawing");

  auto& cell = lib.createCell("inv");
  auto& view = cell.createView(aurora::db::DbViewType::Layout);

  // Two wide rectangles with plenty of spacing — no violations
  using B = aurora::geom::GeomBox;
  (void)view.createRect(layer.id(), B{0, 0, 1000, 1000});   // 1µm × 1µm
  (void)view.createRect(layer.id(), B{2000, 0, 3000, 1000}); // spaced 1µm apart

  aurora::drc_lvs::DrcEngine drc(tech);
  const auto violations = drc.run(view, lib);
  CHECK(violations.empty());
}

static void test_min_width_violation() {
  const auto dir = makeTempDir();
  const auto techPath = dir / "tech_mw.json";
  writeTechJson(techPath);

  aurora::tech::TechDatabase tech;
  CHECK(tech.loadFromJsonFile(techPath));

  aurora::db::DbCellLib lib("testlib");
  auto& layer = lib.createLayer("metal1", "drawing");

  auto& cell = lib.createCell("narrow");
  auto& view = cell.createView(aurora::db::DbViewType::Layout);

  // Shape narrower than 200nm min-width in Y direction
  (void)view.createRect(layer.id(), aurora::geom::GeomBox{0, 0, 1000, 100});

  aurora::drc_lvs::DrcEngine drc(tech);
  const auto violations = drc.run(view, lib);
  bool found = false;
  for (const auto& v : violations)
    if (v.type == aurora::drc_lvs::DrcViolationType::MinWidth) found = true;
  CHECK(found);
}

static void test_min_spacing_violation() {
  const auto dir = makeTempDir();
  const auto techPath = dir / "tech_sp.json";
  writeTechJson(techPath);

  aurora::tech::TechDatabase tech;
  CHECK(tech.loadFromJsonFile(techPath));

  aurora::db::DbCellLib lib("testlib");
  auto& layer = lib.createLayer("metal1", "drawing");

  auto& cell = lib.createCell("close");
  auto& view = cell.createView(aurora::db::DbViewType::Layout);

  // Two rects spaced only 50nm apart (min-spacing = 200nm)
  (void)view.createRect(layer.id(), aurora::geom::GeomBox{0, 0, 500, 500});
  (void)view.createRect(layer.id(), aurora::geom::GeomBox{550, 0, 1050, 500});

  aurora::drc_lvs::DrcEngine drc(tech);
  const auto violations = drc.run(view, lib);
  bool found = false;
  for (const auto& v : violations)
    if (v.type == aurora::drc_lvs::DrcViolationType::MinSpacing) found = true;
  CHECK(found);
}

static void test_lvs_matching() {
  aurora::db::DbCellLib lib("testlib");
  auto& cell = lib.createCell("buf");

  auto& schView = cell.createView(aurora::db::DbViewType::Schematic);
  auto& vdd = schView.createNet("VDD");
  auto& gnd = schView.createNet("GND");
  (void)schView.createPin("VDD", aurora::db::DbPinDirection::Passive, vdd.id());
  (void)schView.createPin("GND", aurora::db::DbPinDirection::Passive, gnd.id());

  auto& layView = cell.createView(aurora::db::DbViewType::Layout);
  auto& lvdd = layView.createNet("VDD");
  auto& lgnd = layView.createNet("GND");
  (void)layView.createPin("VDD", aurora::db::DbPinDirection::Passive, lvdd.id());
  (void)layView.createPin("GND", aurora::db::DbPinDirection::Passive, lgnd.id());

  aurora::drc_lvs::LvsChecker lvs;
  const auto result = lvs.compare(schView, layView, lib);
  CHECK(result.matched);
  CHECK(result.errors.empty());
}

static void test_lvs_mismatch() {
  aurora::db::DbCellLib lib("testlib");
  auto& cell = lib.createCell("mismatch");

  auto& schView = cell.createView(aurora::db::DbViewType::Schematic);
  (void)schView.createNet("VDD");
  (void)schView.createNet("GND");
  (void)schView.createNet("OUT");

  auto& layView = cell.createView(aurora::db::DbViewType::Layout);
  (void)layView.createNet("VDD");
  (void)layView.createNet("GND");
  // "OUT" is missing from layout

  aurora::drc_lvs::LvsChecker lvs;
  const auto result = lvs.compare(schView, layView, lib);
  CHECK(!result.matched);
  CHECK(!result.errors.empty());
}

int main() {
  test_no_violations_compliant();
  test_min_width_violation();
  test_min_spacing_violation();
  test_lvs_matching();
  test_lvs_mismatch();

  if (failures == 0) {
    std::puts("All DRC/LVS tests passed.");
    return 0;
  }
  std::fprintf(stderr, "%d test(s) failed.\n", failures);
  return 1;
}
