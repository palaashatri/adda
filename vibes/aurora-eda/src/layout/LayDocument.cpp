#include "layout/LayDocument.h"

#include <stdexcept>

namespace aurora::layout {

LayDocument::LayDocument(db::DbView& view) : view_(&view) {
  if (view.type() != db::DbViewType::Layout) {
    throw std::invalid_argument("LayDocument requires a layout DbView");
  }
}

db::DbView& LayDocument::view() {
  return *view_;
}

const db::DbView& LayDocument::view() const {
  return *view_;
}

}  // namespace aurora::layout
