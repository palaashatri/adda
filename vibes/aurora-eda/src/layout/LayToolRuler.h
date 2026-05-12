#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <optional>

namespace aurora::layout {

class LayToolRuler : public LayTool {
 public:
  LayToolRuler();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool hasStart() const { return start_.has_value(); }
  [[nodiscard]] geom::GeomPoint startPoint() const { return start_.value_or(geom::GeomPoint{}); }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  std::optional<geom::GeomPoint> start_;
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::layout
