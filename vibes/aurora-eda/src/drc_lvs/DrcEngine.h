#pragma once

#include "drc_lvs/DrcViolation.h"

#include <vector>

namespace aurora::db {
class DbCellLib;
class DbView;
}  // namespace aurora::db

namespace aurora::tech {
class TechDatabase;
}  // namespace aurora::tech

namespace aurora::drc_lvs {

// Runs design-rule checks on a layout view using the tech database for rule values.
class DrcEngine {
 public:
  explicit DrcEngine(const tech::TechDatabase& tech);

  [[nodiscard]] std::vector<DrcViolation> run(const db::DbView& view,
                                               const db::DbCellLib& lib) const;

 private:
  void checkMinWidth(const db::DbView& view, const db::DbCellLib& lib,
                     std::vector<DrcViolation>& out) const;
  void checkMinSpacing(const db::DbView& view, const db::DbCellLib& lib,
                       std::vector<DrcViolation>& out) const;
  void checkNonManhattan(const db::DbView& view, std::vector<DrcViolation>& out) const;

  const tech::TechDatabase& tech_;
};

}  // namespace aurora::drc_lvs
