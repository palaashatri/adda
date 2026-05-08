#pragma once

#include "geom/GeomPoint.h"
#include "layout/LayDocument.h"
#include <memory>

namespace aurora::layout {

class LayTool;

class LayEditorController {
 public:
  explicit LayEditorController(LayDocument& document);
  ~LayEditorController();

  [[nodiscard]] LayDocument& document();
  [[nodiscard]] const LayDocument& document() const;
  [[nodiscard]] geom::DbUnit grid() const;
  [[nodiscard]] double zoom() const;

  void setGrid(geom::DbUnit grid);
  void setZoom(double zoom);
  void setActiveLayerId(db::DbId layerId);

  void setActiveTool(std::unique_ptr<LayTool> tool);
  [[nodiscard]] LayTool* activeTool() const;
  [[nodiscard]] db::DbId activeLayerId() const;

  void mousePress(geom::GeomPoint p);
  void mouseMove(geom::GeomPoint p);
  void mouseRelease(geom::GeomPoint p);

 private:
  LayDocument* document_{nullptr};
  geom::DbUnit grid_{100};
  double zoom_{1.0};
  db::DbId activeLayerId_{db::kInvalidId};
  std::unique_ptr<LayTool> activeTool_;
};

}  // namespace aurora::layout
