#include "tech/TechDatabase.h"

#include <cstdint>
#include <cmath>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <sstream>
#include <utility>

namespace aurora::tech {
namespace {

using Json = nlohmann::json;

[[nodiscard]] geom::DbUnit parseDbUnitValue(const Json& value, std::string_view fieldName) {
  if (value.is_number_integer()) {
    return value.get<geom::DbUnit>();
  }
  if (value.is_number_unsigned()) {
    return static_cast<geom::DbUnit>(value.get<std::uint64_t>());
  }
  if (value.is_number_float()) {
    return static_cast<geom::DbUnit>(std::llround(value.get<double>()));
  }

  std::ostringstream message;
  message << fieldName << " must be numeric";
  throw std::runtime_error(message.str());
}

[[nodiscard]] int optionalIntValue(const Json& object, std::string_view fieldName,
                                   int fallback = -1) {
  const auto it = object.find(std::string(fieldName));
  if (it == object.end()) {
    return fallback;
  }
  if (!it->is_number_integer() && !it->is_number_unsigned()) {
    std::ostringstream message;
    message << fieldName << " must be an integer";
    throw std::runtime_error(message.str());
  }
  return it->get<int>();
}

[[nodiscard]] std::string optionalStringValue(const Json& object, std::string_view fieldName,
                                              std::string fallback = {}) {
  const auto it = object.find(std::string(fieldName));
  if (it == object.end()) {
    return fallback;
  }
  if (!it->is_string()) {
    std::ostringstream message;
    message << fieldName << " must be a string";
    throw std::runtime_error(message.str());
  }
  return it->get<std::string>();
}

[[nodiscard]] geom::DbUnit layerRuleValue(const Json& layer, std::string_view primary,
                                          std::string_view alternate,
                                          std::string_view ruleName) {
  if (layer.contains(std::string(primary))) {
    return parseDbUnitValue(layer.at(std::string(primary)), primary);
  }
  if (layer.contains(std::string(alternate))) {
    return parseDbUnitValue(layer.at(std::string(alternate)), alternate);
  }

  const auto rules = layer.find("rules");
  if (rules != layer.end() && rules->is_object() && rules->contains(std::string(ruleName))) {
    return parseDbUnitValue(rules->at(std::string(ruleName)), ruleName);
  }

  return 0;
}

}  // namespace

bool TechDatabase::loadFromJsonFile(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) {
    lastError_ = "Unable to open tech file: " + path.string();
    return false;
  }

  std::string rawJson{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};

  try {
    const auto root = Json::parse(rawJson);
    if (!root.is_object()) {
      throw std::runtime_error("tech.json root must be an object");
    }

    TechDatabase parsed;
    parsed.rawJson_ = std::move(rawJson);
    parsed.sourcePath_ = std::filesystem::absolute(path);
    parsed.name_ = optionalStringValue(root, "name", path.stem().string());

    if (const auto units = root.find("units"); units != root.end()) {
      if (!units->is_object()) {
        throw std::runtime_error("units must be an object");
      }
      parsed.units_.databaseUnit =
          optionalStringValue(*units, "database_unit",
                              optionalStringValue(*units, "database", parsed.units_.databaseUnit));
      if (units->contains("dbu_per_micron")) {
        if (!units->at("dbu_per_micron").is_number()) {
          throw std::runtime_error("dbu_per_micron must be numeric");
        }
        parsed.units_.dbuPerMicron = units->at("dbu_per_micron").get<double>();
      }
    }

    const auto layers = root.find("layers");
    if (layers == root.end() || !layers->is_array()) {
      throw std::runtime_error("tech.json requires a layers array");
    }

    for (const auto& layerJson : *layers) {
      if (!layerJson.is_object()) {
        throw std::runtime_error("layer entries must be objects");
      }

      TechLayerInfo layer;
      if (layerJson.contains("id")) {
        layer.id = parseDbUnitValue(layerJson.at("id"), "id");
      }
      layer.name = optionalStringValue(layerJson, "name");
      if (layer.name.empty()) {
        throw std::runtime_error("layer.name is required");
      }
      layer.purpose = optionalStringValue(layerJson, "purpose", "drawing");
      layer.color = optionalStringValue(layerJson, "color", layer.color);
      layer.defaultWidth = layerRuleValue(layerJson, "default_width", "min_width", "min_width");
      layer.defaultSpacing =
          layerRuleValue(layerJson, "default_spacing", "min_spacing", "min_spacing");

      if (const auto gds = layerJson.find("gds"); gds != layerJson.end()) {
        if (!gds->is_object()) {
          throw std::runtime_error("layer.gds must be an object");
        }
        layer.gdsLayer = optionalIntValue(*gds, "layer", layer.gdsLayer);
        layer.gdsDatatype = optionalIntValue(*gds, "datatype", layer.gdsDatatype);
      }
      layer.gdsLayer = optionalIntValue(layerJson, "gds_layer", layer.gdsLayer);
      layer.gdsDatatype = optionalIntValue(layerJson, "gds_datatype", layer.gdsDatatype);

      (void)parsed.addLayer(std::move(layer));
    }

    if (const auto rules = root.find("rules"); rules != root.end()) {
      if (!rules->is_array()) {
        throw std::runtime_error("rules must be an array");
      }

      for (const auto& ruleJson : *rules) {
        if (!ruleJson.is_object()) {
          throw std::runtime_error("rule entries must be objects");
        }

        TechRule rule;
        rule.layerName = optionalStringValue(ruleJson, "layer");
        rule.type = optionalStringValue(ruleJson, "type");
        if (rule.layerName.empty() || rule.type.empty()) {
          throw std::runtime_error("rule.layer and rule.type are required");
        }
        if (!ruleJson.contains("value")) {
          throw std::runtime_error("rule.value is required");
        }
        rule.value = parseDbUnitValue(ruleJson.at("value"), "value");
        rule.appliesTo = optionalStringValue(ruleJson, "applies_to");
        parsed.rules_.push_back(std::move(rule));
      }
    }

    parsed.loaded_ = true;
    *this = std::move(parsed);
    return true;
  } catch (const std::exception& error) {
    clear();
    lastError_ = error.what();
    return false;
  }
}

bool TechDatabase::loaded() const {
  return loaded_;
}

const std::string& TechDatabase::name() const {
  return name_;
}

const TechUnits& TechDatabase::units() const {
  return units_;
}

const std::filesystem::path& TechDatabase::sourcePath() const {
  return sourcePath_;
}

const std::string& TechDatabase::rawJson() const {
  return rawJson_;
}

const std::string& TechDatabase::lastError() const {
  return lastError_;
}

const TechLayerInfo& TechDatabase::addLayer(TechLayerInfo layer) {
  if (layer.id == db::kInvalidId) {
    layer.id = nextLayerId_++;
  }

  if (nextLayerId_ <= layer.id) {
    nextLayerId_ = layer.id + 1;
  }

  if (const auto existing = layers_.find(layer.id); existing != layers_.end()) {
    layerNameIndex_.erase(existing->second.name);
  }

  auto [it, inserted] = layers_.insert_or_assign(layer.id, std::move(layer));
  (void)inserted;
  layerNameIndex_[it->second.name] = it->first;
  return it->second;
}

const TechLayerInfo* TechDatabase::findLayerById(db::DbId id) const {
  const auto it = layers_.find(id);
  return it == layers_.end() ? nullptr : &it->second;
}

const TechLayerInfo* TechDatabase::findLayerByName(std::string_view name) const {
  const auto it = layerNameIndex_.find(name);
  return it == layerNameIndex_.end() ? nullptr : findLayerById(it->second);
}

std::vector<db::DbId> TechDatabase::layerIds() const {
  std::vector<db::DbId> ids;
  ids.reserve(layers_.size());
  for (const auto& [id, layer] : layers_) {
    (void)layer;
    ids.push_back(id);
  }
  return ids;
}

const std::vector<TechRule>& TechDatabase::rules() const {
  return rules_;
}

geom::DbUnit TechDatabase::defaultWidthForLayer(std::string_view layerName) const {
  const auto* layer = findLayerByName(layerName);
  return layer == nullptr ? 0 : layer->defaultWidth;
}

geom::DbUnit TechDatabase::defaultSpacingForLayer(std::string_view layerName) const {
  const auto* layer = findLayerByName(layerName);
  return layer == nullptr ? 0 : layer->defaultSpacing;
}

void TechDatabase::clear() {
  sourcePath_.clear();
  rawJson_.clear();
  name_.clear();
  lastError_.clear();
  units_ = {};
  loaded_ = false;
  nextLayerId_ = 1;
  layers_.clear();
  layerNameIndex_.clear();
  rules_.clear();
}

}  // namespace aurora::tech
