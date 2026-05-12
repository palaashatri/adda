#pragma once

#include "layout/LayTool.h"
#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

#include <optional>

namespace aurora::layout {

class LayToolStretch : public LayTool {
 public:
  LayToolStretch();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool isStretching() const { return stretching_; }

 private:
  bool stretching_{false};
  db::DbId targetShapeId_{db::kInvalidId};
  bool stretchLeft_{false}, stretchRight_{false};
  bool stretchBottom_{false}, stretchTop_{false};
  geom::GeomPoint dragStart_;
  geom::GeomPoint cursor_;
};

}  // namespace aurora::layout
