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

 private:
  std::optional<geom::GeomPoint> firstPoint_;
};

}  // namespace aurora::layout
