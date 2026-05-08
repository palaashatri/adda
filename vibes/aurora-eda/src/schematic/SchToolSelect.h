#pragma once

#include "schematic/SchTool.h"
#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

#include <optional>
#include <set>

namespace aurora::schematic {

class SchToolSelect : public SchTool {
 public:
  SchToolSelect();

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseRelease(SchEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(SchEditorController& ctrl, SchKeyEvent key) override;

  [[nodiscard]] const std::set<db::DbId>& selectedInstances() const { return selected_; }
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

}  // namespace aurora::schematic
