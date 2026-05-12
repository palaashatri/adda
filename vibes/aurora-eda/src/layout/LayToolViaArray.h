#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <optional>

namespace aurora::layout {

struct ViaArrayParams {
  int columns = 2;
  int rows = 2;
  geom::DbUnit viaSize = 200;
  geom::DbUnit spacingX = 400;
  geom::DbUnit spacingY = 400;
};

class LayToolViaArray : public LayTool {
 public:
  LayToolViaArray();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool isDrawing() const { return firstPoint_.has_value(); }
  [[nodiscard]] geom::GeomPoint firstPoint() const { return firstPoint_.value_or(geom::GeomPoint{}); }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

  // UI sets this to show a dialog returning via params (or nullopt for cancel)
  std::function<std::optional<ViaArrayParams>()> requestParams;

  ViaArrayParams params;

 private:
  std::optional<geom::GeomPoint> firstPoint_;
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::layout
