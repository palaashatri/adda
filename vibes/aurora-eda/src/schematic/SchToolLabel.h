#pragma once

#include "schematic/SchTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <string>

namespace aurora::schematic {

class SchToolLabel : public SchTool {
 public:
  SchToolLabel();

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(SchEditorController& ctrl, SchKeyEvent key) override;

  // UI sets this to open a dialog returning a label name (or empty for cancel)
  std::function<std::string(geom::GeomPoint)> requestLabelName;

  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::schematic
