#pragma once

#include "pdk/PcellDescriptor.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace aurora::pdk {

// Global registry for native C++ PCells.
class PcellRegistry {
 public:
  // Register a PCell. Overwrites any existing entry with the same name.
  void registerPcell(PcellDescriptor descriptor);

  [[nodiscard]] const PcellDescriptor* find(std::string_view name) const;
  [[nodiscard]] std::vector<std::string> names() const;

  // Invoke a registered PCell.
  // Returns false if the name is not found.
  [[nodiscard]] bool invoke(std::string_view name, db::DbView& layoutView,
                             const db::DbCellLib& lib, const tech::TechDatabase& tech,
                             const ParamMap& params) const;

 private:
  std::map<std::string, PcellDescriptor, std::less<>> pcells_;
};

}  // namespace aurora::pdk
