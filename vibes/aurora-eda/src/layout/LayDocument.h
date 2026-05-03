#pragma once

#include "db/DbView.h"

namespace aurora::layout {

class LayDocument {
 public:
  explicit LayDocument(db::DbView& view);

  [[nodiscard]] db::DbView& view();
  [[nodiscard]] const db::DbView& view() const;

 private:
  db::DbView* view_{nullptr};
};

}  // namespace aurora::layout
