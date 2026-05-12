#pragma once

#include "schematic/SchTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <optional>
#include <string>

namespace aurora::schematic {

struct PinDefinition {
  std::string name;
  std::string direction; // "input", "output", "inout"
};

class SchToolSymbolPin : public SchTool {
 public:
  SchToolSymbolPin();

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(SchEditorController& ctrl, SchKeyEvent key) override;

  std::function<std::optional<PinDefinition>()> requestPin;

  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::schematic
