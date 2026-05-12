#pragma once

#include "geom/GeomBox.h"

#include <map>
#include <string>
#include <vector>

namespace aurora::db {
class DbCellLib;
class DbView;
}

namespace aurora::drc_lvs {

struct RecognizedDevice {
  std::string type; // "NMOS", "PMOS", "RES", "CAP"
  geom::GeomBox boundingBox;
  std::map<std::string, double> params; // W, L, fingers, etc.
};

struct LvsResult {
  bool matched{false};
  std::vector<std::string> errors;
  std::vector<RecognizedDevice> recognizedDevices; // E4
};

class LvsChecker {
 public:
  [[nodiscard]] LvsResult compare(const db::DbView& schView, const db::DbView& layView,
                                   const db::DbCellLib& lib) const;

  // E4: Recognize devices from layout geometry
  [[nodiscard]] std::vector<RecognizedDevice> recognizeDevices(const db::DbView& view,
                                                                const db::DbCellLib& lib) const;
};

}  // namespace aurora::drc_lvs
