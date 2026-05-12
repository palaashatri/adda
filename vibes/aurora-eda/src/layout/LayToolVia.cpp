#include "layout/LayToolVia.h"
#include "layout/LayEditorController.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

namespace aurora::layout {

LayToolVia::LayToolVia() : LayTool("Via") {}

void LayToolVia::mousePress(LayEditorController& ctrl, geom::GeomPoint p) {
  if (!requestParams) return;
  auto pr = requestParams();
  if (!pr) return;

  auto& view = ctrl.document().view();
  auto layerId = ctrl.activeLayerId();

  // Contact/via square
  geom::GeomBox contact{p.x - pr->width / 2, p.y - pr->height / 2,
                         p.x + pr->width / 2, p.y + pr->height / 2};
  (void)view.createRect(layerId, contact);

  // Enclosure layers (if available, use adjacent layer IDs)
  geom::GeomBox enc1{p.x - pr->width / 2 - pr->encLayer1,
                     p.y - pr->height / 2 - pr->encLayer1,
                     p.x + pr->width / 2 + pr->encLayer1,
                     p.y + pr->height / 2 + pr->encLayer1};

  // Try to find a second layer if available
  auto& lib_ = ctrl.document().view();
  (void)lib_;
  // Create enclosure on the same layer as visual marker
  (void)view.createRect(layerId, enc1);
}

void LayToolVia::mouseMove(LayEditorController& ctrl, geom::GeomPoint p) { cursor_ = p; }
void LayToolVia::keyPress(LayEditorController& ctrl, int qtKey) { (void)ctrl; (void)qtKey; }

}  // namespace aurora::layout
