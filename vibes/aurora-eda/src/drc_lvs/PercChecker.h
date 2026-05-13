#pragma once

#include "drc_lvs/DrcViolation.h"

#include <string>
#include <vector>

namespace aurora::db {
class DbView;
class DbCellLib;
}

namespace aurora::tech {
class TechDatabase;
}

namespace aurora::drc_lvs {

// E11 — PERC (Programmable Electrical Rule Check) / power integrity.
// Checks IR-drop estimates, current density vs metal width, simple electromigration.
struct PercOptions {
  double supplyVoltage{1.8};          // V
  double maxIRDropVolts{0.1};         // V
  double sheetResistanceOhm{0.08};    // Ω/sq (metal)
  double maxCurrentDensity{1.0};      // mA/µm width
  std::string powerNet{"VDD"};
  std::string groundNet{"VSS"};
  double estimatedCurrentMa{1.0};     // mA total drawn
};

struct PercResult {
  std::vector<DrcViolation> violations;
  double estimatedIRDrop{0.0};
  double worstCurrentDensity{0.0};
};

class PercChecker {
 public:
  explicit PercChecker(const tech::TechDatabase& tech, PercOptions opts = {});

  [[nodiscard]] PercResult run(const db::DbView& view, const db::DbCellLib& lib) const;

 private:
  const tech::TechDatabase& tech_;
  PercOptions opts_;
};

}  // namespace aurora::drc_lvs
