#pragma once

#include "schematic/SchTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace aurora::schematic {

struct StimulusParams {
  std::string type; // "vdc", "idc", "vpulse", "vsin"
  std::map<std::string, std::string> values;
};

class SchToolStimulus : public SchTool {
 public:
  SchToolStimulus();

  void mousePress(SchEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(SchEditorController& ctrl, SchKeyEvent key) override;

  // UI sets this to show a dialog returning params (or nullopt for cancel)
  std::function<std::optional<StimulusParams>()> requestParams;

  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::schematic
