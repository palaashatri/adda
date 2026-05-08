#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <vector>

namespace aurora::layout {

class LayToolPolygon : public LayTool {
 public:
  LayToolPolygon();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool isDrawing() const { return !points_.empty(); }
  [[nodiscard]] const std::vector<geom::GeomPoint>& points() const { return points_; }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  void commitPolygon(LayEditorController& ctrl);
  void cancel();

  std::vector<geom::GeomPoint> points_;
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::layout
