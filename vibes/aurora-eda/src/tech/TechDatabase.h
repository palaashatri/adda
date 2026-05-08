#pragma once

#include "db/DbLayer.h"
#include "geom/GeomPoint.h"

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace aurora::tech {

struct TechUnits {
  std::string databaseUnit{"nm"};
  double dbuPerMicron{1000.0};
};

struct TechLayerInfo {
  db::DbId id{db::kInvalidId};
  std::string name;
  std::string purpose;
  std::string color{"#808080"};
  int gdsLayer{-1};
  int gdsDatatype{-1};
  geom::DbUnit defaultWidth{0};
  geom::DbUnit defaultSpacing{0};
};

struct TechRule {
  std::string layerName;
  std::string type;
  geom::DbUnit value{0};
  std::string appliesTo;
};

class TechDatabase {
 public:
  [[nodiscard]] bool loadFromJsonFile(const std::filesystem::path& path);
  [[nodiscard]] bool loaded() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] const TechUnits& units() const;
  [[nodiscard]] const std::filesystem::path& sourcePath() const;
  [[nodiscard]] const std::string& rawJson() const;
  [[nodiscard]] const std::string& lastError() const;

  [[nodiscard]] const TechLayerInfo& addLayer(TechLayerInfo layer);
  [[nodiscard]] const TechLayerInfo* findLayerById(db::DbId id) const;
  [[nodiscard]] const TechLayerInfo* findLayerByName(std::string_view name) const;
  [[nodiscard]] std::vector<db::DbId> layerIds() const;
  [[nodiscard]] const std::vector<TechRule>& rules() const;
  [[nodiscard]] geom::DbUnit defaultWidthForLayer(std::string_view layerName) const;
  [[nodiscard]] geom::DbUnit defaultSpacingForLayer(std::string_view layerName) const;

  void clear();

 private:
  std::filesystem::path sourcePath_;
  std::string rawJson_;
  std::string name_;
  std::string lastError_;
  TechUnits units_;
  bool loaded_{false};
  db::DbId nextLayerId_{1};
  std::map<db::DbId, TechLayerInfo> layers_;
  std::map<std::string, db::DbId, std::less<>> layerNameIndex_;
  std::vector<TechRule> rules_;
};

}  // namespace aurora::tech
