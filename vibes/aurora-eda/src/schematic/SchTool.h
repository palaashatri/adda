#pragma once

#include "geom/GeomPoint.h"
#include <string>
#include <string_view>

namespace aurora::schematic {

class SchEditorController;

enum class SchKeyEvent { Escape, Enter, Delete, Other };

class SchTool {
 public:
  explicit SchTool(std::string_view name) : name_(name) {}
  virtual ~SchTool();

  [[nodiscard]] std::string_view name() const { return name_; }

  virtual void mousePress(SchEditorController& ctrl, geom::GeomPoint p) {}
  virtual void mouseMove(SchEditorController& ctrl, geom::GeomPoint p) {}
  virtual void mouseRelease(SchEditorController& ctrl, geom::GeomPoint p) {}
  virtual void keyPress(SchEditorController& ctrl, SchKeyEvent key) {}
  virtual void activate(SchEditorController& ctrl) {}
  virtual void deactivate(SchEditorController& ctrl) {}

 private:
  std::string name_;
};

}  // namespace aurora::schematic
