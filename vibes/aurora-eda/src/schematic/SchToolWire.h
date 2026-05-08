#pragma once

#include "schematic/SchTool.h"
#include "geom/GeomPoint.h"

namespace aurora::schematic {

class SchToolWire : public SchTool {
 public:
  SchToolWire();

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(SchEditorController& ctrl, SchKeyEvent key) override;
  void deactivate(SchEditorController& ctrl) override;

  [[nodiscard]] bool isDrawing() const { return drawing_; }
  [[nodiscard]] geom::GeomPoint startPoint() const { return startPoint_; }
  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  bool drawing_{false};
  geom::GeomPoint startPoint_{};
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::schematic
