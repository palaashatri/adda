#pragma once

#include "schematic/SchTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <optional>
#include <string>

namespace aurora::schematic {

class SchToolProbe : public SchTool {
 public:
  SchToolProbe();

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(SchEditorController& ctrl, SchKeyEvent key) override;

  std::function<std::optional<std::string>()> requestType; // "vprobe" or "iprobe"

  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::schematic
