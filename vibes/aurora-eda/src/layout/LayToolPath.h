#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <vector>

namespace aurora::layout {

class LayToolPath : public LayTool {
 public:
  LayToolPath();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] bool isDrawing() const { return !points_.empty(); }
  [[nodiscard]] const std::vector<geom::GeomPoint>& points() const { return points_; }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }
  [[nodiscard]] geom::DbUnit pathWidth() const { return width_; }
  void setPathWidth(geom::DbUnit w) { width_ = w; }

 private:
  void commitPath(LayEditorController& ctrl);
  void cancel();

  std::vector<geom::GeomPoint> points_;
  geom::GeomPoint cursor_{};
  geom::DbUnit width_{100};
};

}  // namespace aurora::layout
