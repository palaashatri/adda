#pragma once

#include "drc_lvs/DrcViolation.h"

#include <optional>
#include <vector>

namespace aurora::db {
class DbCellLib;
class DbView;
}

namespace aurora::tech {
class TechDatabase;
}

namespace aurora::drc_lvs {

struct DrcOptions {
  bool checkMinWidth{true};
  bool checkMinSpacing{true};
  bool checkNonManhattan{true};
  bool checkAntenna{true};      // E8
  bool checkDensity{true};      // E9
  bool checkErc{true};          // E10
  bool hierarchical{false};     // E2
  std::optional<geom::GeomBox> areaOnly; // E3
};

class DrcEngine {
 public:
  explicit DrcEngine(const tech::TechDatabase& tech);

  [[nodiscard]] std::vector<DrcViolation> run(const db::DbView& view,
                                               const db::DbCellLib& lib,
                                               const DrcOptions& opts = {}) const;

 private:
  void checkMinWidth(const db::DbView& view, const db::DbCellLib& lib,
                     std::vector<DrcViolation>& out) const;
  void checkMinSpacing(const db::DbView& view, const db::DbCellLib& lib,
                       std::vector<DrcViolation>& out) const;
  void checkNonManhattan(const db::DbView& view, std::vector<DrcViolation>& out) const;

  // E10: Electrical rule checks
  void checkErc(const db::DbView& view, const db::DbCellLib& lib,
                std::vector<DrcViolation>& out) const;

  // E8: Antenna rule checking
  void checkAntenna(const db::DbView& view, const db::DbCellLib& lib,
                    std::vector<DrcViolation>& out) const;

  // E9: Density checking
  void checkDensity(const db::DbView& view, const db::DbCellLib& lib,
                    std::vector<DrcViolation>& out) const;

  const tech::TechDatabase& tech_;
};

}  // namespace aurora::drc_lvs
