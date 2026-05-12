#pragma once

#include <string>
#include <vector>

namespace aurora::db {
class DbCellLib;
class DbView;
}

namespace aurora::tech {
class TechDatabase;
}

namespace aurora::drc_lvs {

struct ParasiticCapacitor {
  std::string layer1;
  std::string layer2;
  double capacitance; // fF
  double area;        // nm²
};

struct ParasiticResistor {
  std::string layer;
  double resistance;  // ohms
  double length;      // nm
};

struct ParasiticResult {
  std::vector<ParasiticCapacitor> caps;
  std::vector<ParasiticResistor> resistors;
  bool extracted{false};
};

class ParasiticExtractor {
 public:
  explicit ParasiticExtractor(const tech::TechDatabase& tech);

  [[nodiscard]] ParasiticResult extract(const db::DbView& view,
                                         const db::DbCellLib& lib) const;
 private:
  const tech::TechDatabase& tech_;
};

}  // namespace aurora::drc_lvs
