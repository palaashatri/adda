#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace aurora::pdk {

// F3 — Component Description Format: typed parameters with units, prompts, choices.
enum class CdfParamType { Integer, Float, String, Boolean, Choice };

struct CdfParam {
  std::string name;
  CdfParamType type{CdfParamType::String};
  std::string defaultValue;
  std::string unit;
  std::string prompt;
  std::vector<std::string> choices;       // For Choice-type
  std::function<bool(const std::string&)> validator;  // F12 callback
  std::function<std::string(const std::map<std::string,std::string>&)> derive;  // derive from others
};

struct CdfDescriptor {
  std::string componentName;
  std::vector<CdfParam> params;

  [[nodiscard]] const CdfParam* find(std::string_view name) const;
  [[nodiscard]] bool validate(const std::map<std::string, std::string>& params,
                              std::vector<std::string>* errors = nullptr) const;
  // Apply derive callbacks; mutates the map in-place.
  void applyDerives(std::map<std::string, std::string>& params) const;
};

class CdfRegistry {
 public:
  void registerCdf(CdfDescriptor desc);
  [[nodiscard]] const CdfDescriptor* find(std::string_view name) const;
  [[nodiscard]] std::vector<std::string> names() const;

 private:
  std::map<std::string, CdfDescriptor, std::less<>> cdfs_;
};

}  // namespace aurora::pdk
