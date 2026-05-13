#include "pdk/PcellEvalCache.h"

#include "db/DbView.h"

namespace aurora::pdk {

std::string PcellEvalCache::hashParams(const ParamMap& params) {
  std::string h;
  for (const auto& [k, v] : params) {
    h += k;
    h += '=';
    h += v;
    h += ';';
  }
  return h;
}

bool PcellEvalCache::evaluate(const PcellDescriptor& desc, db::DbView& view,
                               const db::DbCellLib& lib, const tech::TechDatabase& tech,
                               const ParamMap& params) {
  const std::string key = desc.name;
  const std::string h = hashParams(params);
  auto it = entries_.find(key);
  if (it != entries_.end() && it->second.valid && it->second.paramHash == h) {
    return true;  // cache hit
  }
  const auto countBefore = view.shapeIds().size();
  desc.generate(view, lib, tech, params);
  const auto countAfter = view.shapeIds().size();
  entries_[key] = {h, countAfter - countBefore, true};
  return false;
}

void PcellEvalCache::invalidate(std::string_view pcellName) {
  auto it = entries_.find(pcellName);
  if (it != entries_.end()) it->second.valid = false;
}

void PcellEvalCache::clear() { entries_.clear(); }

}  // namespace aurora::pdk
