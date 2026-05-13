#pragma once

#include "pdk/PcellDescriptor.h"

#include <map>
#include <string>

namespace aurora::pdk {

// F4 — PCell evaluation engine with caching.
// Caches the generated shape count and a hash of inputs; invalidates on param change.
struct CacheEntry {
  std::string paramHash;
  std::size_t shapeCount{0};
  bool valid{false};
};

class PcellEvalCache {
 public:
  // Returns true if cache hit (and skipped regen); false if a fresh eval was performed.
  bool evaluate(const PcellDescriptor& desc, db::DbView& view,
                const db::DbCellLib& lib, const tech::TechDatabase& tech,
                const ParamMap& params);

  void invalidate(std::string_view pcellName);
  void clear();
  [[nodiscard]] std::size_t size() const { return entries_.size(); }

 private:
  static std::string hashParams(const ParamMap& params);
  std::map<std::string, CacheEntry, std::less<>> entries_;
};

}  // namespace aurora::pdk
