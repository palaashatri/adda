#include "pdk/Cdf.h"

namespace aurora::pdk {

const CdfParam* CdfDescriptor::find(std::string_view name) const {
  for (const auto& p : params) {
    if (p.name == name) return &p;
  }
  return nullptr;
}

bool CdfDescriptor::validate(const std::map<std::string, std::string>& params,
                              std::vector<std::string>* errors) const {
  bool ok = true;
  for (const auto& p : this->params) {
    auto it = params.find(p.name);
    if (it == params.end()) continue;  // optional, use default later
    const std::string& v = it->second;

    // Type-based validation.
    bool typeOk = true;
    try {
      switch (p.type) {
        case CdfParamType::Integer: (void)std::stoll(v); break;
        case CdfParamType::Float:   (void)std::stod(v); break;
        case CdfParamType::Boolean:
          typeOk = (v == "true" || v == "false" || v == "0" || v == "1");
          break;
        case CdfParamType::Choice: {
          bool found = false;
          for (const auto& c : p.choices) if (c == v) { found = true; break; }
          typeOk = found;
          break;
        }
        case CdfParamType::String: break;
      }
    } catch (...) { typeOk = false; }

    if (!typeOk) {
      ok = false;
      if (errors) errors->push_back("Invalid type for " + p.name + ": '" + v + "'");
    }
    if (p.validator && !p.validator(v)) {
      ok = false;
      if (errors) errors->push_back("Validator failed for " + p.name);
    }
  }
  return ok;
}

void CdfDescriptor::applyDerives(std::map<std::string, std::string>& params) const {
  for (const auto& p : this->params) {
    if (p.derive) {
      params[p.name] = p.derive(params);
    }
  }
}

void CdfRegistry::registerCdf(CdfDescriptor desc) {
  const std::string key = desc.componentName;
  cdfs_[key] = std::move(desc);
}

const CdfDescriptor* CdfRegistry::find(std::string_view name) const {
  auto it = cdfs_.find(name);
  return it != cdfs_.end() ? &it->second : nullptr;
}

std::vector<std::string> CdfRegistry::names() const {
  std::vector<std::string> out;
  out.reserve(cdfs_.size());
  for (const auto& [n, _] : cdfs_) out.push_back(n);
  return out;
}

}  // namespace aurora::pdk
