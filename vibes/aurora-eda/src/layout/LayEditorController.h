#pragma once

#include "geom/GeomPoint.h"
#include "layout/LayDocument.h"

namespace aurora::layout {

class LayEditorController {
 public:
  explicit LayEditorController(LayDocument& document);

  [[nodiscard]] LayDocument& document();
  [[nodiscard]] const LayDocument& document() const;
  [[nodiscard]] geom::DbUnit grid() const;
  [[nodiscard]] double zoom() const;

  void setGrid(geom::DbUnit grid);
  void setZoom(double zoom);

 private:
  LayDocument* document_{nullptr};
  geom::DbUnit grid_{100};
  double zoom_{1.0};
};

}  // namespace aurora::layout
