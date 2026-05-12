#pragma once

#include "layout/LayTool.h"
#include "geom/GeomPoint.h"

#include <functional>
#include <optional>

namespace aurora::layout {

struct ViaParams {
  geom::DbUnit width = 200;
  geom::DbUnit height = 200;
  geom::DbUnit encLayer1 = 100;
  geom::DbUnit encLayer2 = 100;
};

class LayToolVia : public LayTool {
 public:
  LayToolVia();

  void mousePress(LayEditorController& ctrl, geom::GeomPoint p) override;
  void mouseMove(LayEditorController& ctrl, geom::GeomPoint p) override;
  void keyPress(LayEditorController& ctrl, int qtKey) override;

  std::function<std::optional<ViaParams>()> requestParams;

  [[nodiscard]] geom::GeomPoint cursor() const { return cursor_; }

 private:
  geom::GeomPoint cursor_{};
};

}  // namespace aurora::layout
