#pragma once

#include "geom/GeomPoint.h"
#include "schematic/SchDocument.h"
#include "schematic/SchTool.h"

#include <memory>
#include <string>

namespace aurora::schematic {

class SchEditorController {
 public:
  explicit SchEditorController(SchDocument& document);
  ~SchEditorController();

  [[nodiscard]] SchDocument& document();
  [[nodiscard]] const SchDocument& document() const;
  [[nodiscard]] geom::DbUnit grid() const;
  void setGrid(geom::DbUnit grid);

  void setActiveTool(std::unique_ptr<SchTool> tool);
  [[nodiscard]] SchTool* activeTool() const { return activeTool_.get(); }

  void mousePress(geom::GeomPoint p);
  void mouseMove(geom::GeomPoint p);
  void mouseRelease(geom::GeomPoint p);
  void keyPress(SchKeyEvent key);

  [[nodiscard]] geom::GeomPoint snap(geom::GeomPoint p) const;
  [[nodiscard]] std::string nextNetName();
  [[nodiscard]] std::string nextInstanceName();

 private:
  SchDocument* document_{nullptr};
  geom::DbUnit grid_{100};
  std::unique_ptr<SchTool> activeTool_;
  int netCounter_{0};
  int instanceCounter_{0};
};

}  // namespace aurora::schematic
