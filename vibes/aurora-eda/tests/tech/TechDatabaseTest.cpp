#include "tech/TechDatabase.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

[[nodiscard]] std::filesystem::path tempFilePath(std::string_view stem) {
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() /
         (std::string(stem) + "_" + std::to_string(now) + ".json");
}

}  // namespace

int main() {
  const auto techPath = tempFilePath("aurora_tech");
  {
    std::ofstream out(techPath);
    out << R"json({
      "name": "demo45",
      "units": {
        "database_unit": "nm",
        "dbu_per_micron": 1000
      },
      "layers": [
        {
          "id": 1,
          "name": "metal1",
          "purpose": "drawing",
          "color": "#3fbf7f",
          "gds": {"layer": 68, "datatype": 20},
          "rules": {"min_width": 140, "min_spacing": 140}
        },
        {
          "name": "poly",
          "purpose": "drawing",
          "gds_layer": 66,
          "gds_datatype": 20,
          "default_width": 120,
          "default_spacing": 180
        }
      ],
      "rules": [
        {"layer": "metal1", "type": "width", "value": 140},
        {"layer": "metal1", "type": "spacing", "value": 140}
      ]
    })json";
  }

  aurora::tech::TechDatabase tech;
  assert(tech.loadFromJsonFile(techPath));
  assert(tech.loaded());
  assert(tech.name() == "demo45");
  assert(tech.units().databaseUnit == "nm");
  assert(tech.units().dbuPerMicron == 1000.0);
  assert(tech.layerIds().size() == 2);
  assert(tech.rules().size() == 2);

  const auto* metal1 = tech.findLayerByName("metal1");
  assert(metal1 != nullptr);
  assert(metal1->id == 1);
  assert(metal1->gdsLayer == 68);
  assert(metal1->gdsDatatype == 20);
  assert(metal1->defaultWidth == 140);
  assert(metal1->defaultSpacing == 140);

  const auto* poly = tech.findLayerByName("poly");
  assert(poly != nullptr);
  assert(poly->defaultWidth == 120);
  assert(poly->defaultSpacing == 180);
  assert(tech.defaultWidthForLayer("poly") == 120);
  assert(tech.defaultSpacingForLayer("poly") == 180);

  const auto invalidPath = tempFilePath("aurora_invalid_tech");
  {
    std::ofstream out(invalidPath);
    out << R"json({"name": "bad"})json";
  }

  assert(!tech.loadFromJsonFile(invalidPath));
  assert(!tech.loaded());
  assert(!tech.lastError().empty());

  std::filesystem::remove(techPath);
  std::filesystem::remove(invalidPath);
  return 0;
}
