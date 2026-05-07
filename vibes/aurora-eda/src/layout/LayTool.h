#pragma once

#include "geom/GeomPoint.h"
#include <string>

namespace aurora::layout {

class LayEditorController;

class LayTool {
 public:
  explicit LayTool(std::string name = {});
  virtual ~LayTool() = default;

  [[nodiscard]] const std::string& name() const;

  virtual void mousePress(LayEditorController& ctrl, geom::GeomPoint p) {}
  virtual void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) {}
  virtual void mouseRelease(LayEditorController& ctrl, geom::GeomPoint p) {}

 private:
  std::string name_;
};

}  // namespace aurora::layout
