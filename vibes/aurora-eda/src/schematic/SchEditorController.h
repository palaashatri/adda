#pragma once

#include "geom/GeomPoint.h"
#include "schematic/SchDocument.h"

namespace aurora::schematic {

class SchEditorController {
 public:
  explicit SchEditorController(SchDocument& document);

  [[nodiscard]] SchDocument& document();
  [[nodiscard]] const SchDocument& document() const;
  [[nodiscard]] geom::DbUnit grid() const;
  void setGrid(geom::DbUnit grid);

 private:
  SchDocument* document_{nullptr};
  geom::DbUnit grid_{100};
};

}  // namespace aurora::schematic
