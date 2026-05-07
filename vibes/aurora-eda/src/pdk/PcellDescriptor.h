#pragma once

#include <functional>
#include <map>
#include <string>

namespace aurora::db {
class DbView;
class DbCellLib;
}  // namespace aurora::db

namespace aurora::tech {
class TechDatabase;
}  // namespace aurora::tech

namespace aurora::pdk {

using ParamMap = std::map<std::string, std::string>;

// Signature of a native C++ PCell generator.
// The generator populates the given layout view with shapes.
using PcellGenerator = std::function<void(db::DbView& layoutView, const db::DbCellLib& lib,
                                          const tech::TechDatabase& tech,
                                          const ParamMap& params)>;

struct PcellDescriptor {
  std::string      name;
  ParamMap         defaultParams;
  PcellGenerator   generate;
};

}  // namespace aurora::pdk
