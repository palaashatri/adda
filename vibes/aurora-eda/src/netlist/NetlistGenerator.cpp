#include "netlist/NetlistGenerator.h"

#include <sstream>

namespace aurora::netlist {

std::string NetlistGenerator::generateSpice(const db::DbCell& cell, const db::DbView& view) const {
  std::ostringstream out;
  out << "* aurora-eda generated SPICE skeleton\n";
  out << "* cell=" << cell.name() << " view=" << db::toString(view.type()) << "\n";
  out << ".end\n";
  return out.str();
}

}  // namespace aurora::netlist
