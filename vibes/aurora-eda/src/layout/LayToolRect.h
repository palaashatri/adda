#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <optional>

namespace aurora::layout {

class LayToolRect : public LayTool {
 public:
  LayToolRect();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool isDrawing() const { return firstPoint_.has_value(); }
  [[nodiscard]] geom::GeomPoint firstPoint() const { return firstPoint_.value_or(geom::GeomPoint{}); }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  std::optional<geom::GeomPoint> firstPoint_;
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::layout
