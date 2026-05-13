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
  [[nodiscard]] bool busMode() const { return busMode_; }
  void setBusMode(bool b) { busMode_ = b; }
  void setBusWidth(int msb, int lsb) { busMsb_ = msb; busLsb_ = lsb; }
  [[nodiscard]] int busMsb() const { return busMsb_; }
  [[nodiscard]] int busLsb() const { return busLsb_; }

 private:
  bool drawing_{false};
  geom::GeomPoint startPoint_{};
  geom::GeomPoint cursor_{};
  bool busMode_{false};
  int busMsb_{7};
  int busLsb_{0};
};

}  // namespace aurora::schematic
