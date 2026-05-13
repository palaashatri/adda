#include "layout/ImportWizard.h"

#include "layout/LayCifIo.h"
#include "layout/LayDefReader.h"
#include "layout/LayGdsReader.h"
#include "layout/LayLefReader.h"
#include "layout/LayOasisReader.h"
#include "netlist/SpiceImporter.h"
#include "netlist/VerilogImporter.h"

namespace aurora::layout {

bool ImportWizard::runAuto(db::DbCellLib& lib, const ImportSelection& sel) {
  using F = ImportSelection::Format;
  F fmt = sel.format;
  if (fmt == F::Auto) {
    const auto ext = sel.inputPath.extension().string();
    if      (ext == ".gds" || ext == ".gdsii") fmt = F::Gds;
    else if (ext == ".lef")                    fmt = F::Lef;
    else if (ext == ".def")                    fmt = F::Def;
    else if (ext == ".v" || ext == ".vh")     fmt = F::Verilog;
    else if (ext == ".sp" || ext == ".spice" || ext == ".cdl") fmt = F::Spice;
    else if (ext == ".oas")                    fmt = F::Oasis;
    else if (ext == ".cif")                    fmt = F::Cif;
    else return false;
  }
  switch (fmt) {
    case F::Gds:     return LayGdsReader{}.read(lib, sel.inputPath);
    case F::Lef:     return LayLefReader{}.read(lib, sel.inputPath);
    case F::Def:     return LayDefReader{}.read(lib, sel.inputPath);
    case F::Verilog: return netlist::VerilogImporter{}.importFile(lib, sel.inputPath);
    case F::Spice:   { netlist::SpiceImporter imp; return imp.importFile(sel.inputPath, lib); }
    case F::Oasis:   return LayOasisReader{}.read(lib, sel.inputPath);
    case F::Cif:     return LayCifIo{}.read(lib, sel.inputPath);
    default:         return false;
  }
}

}  // namespace aurora::layout
