#pragma once

#include "schematic/SchTool.h"
#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

namespace aurora::schematic {

class SchToolInstance : public SchTool {
 public:
  explicit SchToolInstance(db::DbId masterCellId);

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;

  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }
  [[nodiscard]] db::DbId masterCellId() const { return masterCellId_; }

 private:
  db::DbId masterCellId_{db::kInvalidId};
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::schematic
