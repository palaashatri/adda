#include "pdk/PcellRegistry.h"

namespace aurora::pdk {

void PcellRegistry::registerPcell(PcellDescriptor descriptor) {
  const std::string key = descriptor.name;
  pcells_[key] = std::move(descriptor);
}

const PcellDescriptor* PcellRegistry::find(std::string_view name) const {
  const auto it = pcells_.find(name);
  return it != pcells_.end() ? &it->second : nullptr;
}

std::vector<std::string> PcellRegistry::names() const {
  std::vector<std::string> result;
  result.reserve(pcells_.size());
  for (const auto& [name, _] : pcells_) result.push_back(name);
  return result;
}

bool PcellRegistry::invoke(std::string_view name, db::DbView& layoutView,
                             const db::DbCellLib& lib, const tech::TechDatabase& tech,
                             const ParamMap& params) const {
  const auto* desc = find(name);
  if (!desc) return false;

  // Merge defaults then override with caller-supplied params
  ParamMap merged = desc->defaultParams;
  for (const auto& [k, v] : params) merged[k] = v;

  desc->generate(layoutView, lib, tech, merged);
  return true;
}

}  // namespace aurora::pdk
