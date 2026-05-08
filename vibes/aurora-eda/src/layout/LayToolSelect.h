#pragma once

#include "layout/LayTool.h"
#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

#include <optional>
#include <set>

namespace aurora::layout {

class LayToolSelect : public LayTool {
 public:
  LayToolSelect();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  [[nodiscard]] const std::set<db::DbId>& selectedShapes() const { return selected_; }
  [[nodiscard]] bool isRubberBanding() const { return rubberBand_; }
  [[nodiscard]] geom::GeomPoint rubberBandStart() const { return rbStart_; }
  [[nodiscard]] geom::GeomPoint rubberBandEnd() const { return rbEnd_; }
  void clearSelection() { selected_.clear(); }

 private:
  std::set<db::DbId> selected_;
  std::optional<geom::GeomPoint> pressPoint_;
  geom::GeomPoint rbStart_{}, rbEnd_{};
  bool rubberBand_{false};
};

}  // namespace aurora::layout
