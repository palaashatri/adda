#include "core/CoreApp.h"
#include "db/DbCellLib.h"

#include <cassert>

int main() {
  aurora::core::CoreApp app;
  assert(app.initialize());
  assert(app.initialized());

  aurora::db::DbCellLib lib{"worklib"};
  auto& metal1 = lib.createLayer("metal1", "drawing");
  metal1.setColor("#3fbf7f");
  metal1.setGdsMapping(68, 20);

  auto& cell = lib.createCell("rc_filter");
  auto& schematic = cell.createView(aurora::db::DbViewType::Schematic);
  auto& layout = cell.createView(aurora::db::DbViewType::Layout);

  auto& net = schematic.createNet("out");
  auto& pin = schematic.createPin("OUT", aurora::db::DbPinDirection::Output, net.id());
  assert(net.pinIds().size() == 1);
  assert(net.pinIds().front() == pin.id());

  auto& rect = layout.createRect(metal1.id(), aurora::geom::GeomBox{0, 0, 1000, 500});
  assert(rect.layerId() == metal1.id());
  assert(layout.shapeIds().size() == 1);
  assert(lib.findCell("rc_filter") == &cell);

  app.shutdown();
  assert(!app.initialized());
  return 0;
}
