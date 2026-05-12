#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <optional>

namespace aurora::layout {

struct GuardRingParams {
  geom::DbUnit ringWidth = 400;
  geom::DbUnit spacing = 200;
};

class LayToolGuardRing : public LayTool {
 public:
  LayToolGuardRing();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool isDrawing() const { return firstPoint_.has_value(); }
  [[nodiscard]] geom::GeomPoint firstPoint() const { return firstPoint_.value_or(geom::GeomPoint{}); }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

  std::function<std::optional<GuardRingParams>()> requestParams;

 private:
  std::optional<geom::GeomPoint> firstPoint_;
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::layout
