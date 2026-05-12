#include "layout/LayDefReader.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"

#include <cassert>
#include <filesystem>
#include <fstream>

static void testDefImport() {
  const auto path = std::filesystem::temp_directory_path() / "aurora_def_import.def";
  {
    std::ofstream f(path);
    f << "VERSION 5.8 ;\n";
    f << "DIVIDERCHAR \"/\" ;\n";
    f << "DESIGN mydesign ;\n";
    f << "UNITS DISTANCE MICRONS 1000 ;\n";
    f << "COMPONENTS 2 ;\n";
    f << "  - I1 INV + PLACED ( 5.0 6.0 ) N ;\n";
    f << "  - I2 INV + PLACED ( 15.0 6.0 ) N ;\n";
    f << "END COMPONENTS\n";
    f << "NETS 2 ;\n";
    f << "  A ( PIN A ) ( I1 A ) ;\n";
    f << "  Y ( I1 Y ) ( I2 A ) ;\n";
    f << "END NETS\n";
    f << "END DESIGN\n";
  }

  aurora::db::DbCellLib lib("test");
  aurora::layout::LayDefReader reader;
  const bool ok = reader.read(lib, path);
  assert(ok && "read must succeed");
  std::filesystem::remove(path);

  const auto* cell = lib.findCell("mydesign");
  assert(cell != nullptr && "cell must exist");
  const auto* view = cell->findView(aurora::db::DbViewType::Layout);
  assert(view != nullptr && "layout view must exist");
  assert(view->instanceIds().size() == 2 && "must have 2 instances");
  assert(view->netIds().size() == 2 && "must have 2 nets");
}

int main() {
  testDefImport();
  return 0;
}
