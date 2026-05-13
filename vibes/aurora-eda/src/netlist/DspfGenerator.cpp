#include "netlist/DspfGenerator.h"

#include "drc_lvs/ParasiticExtractor.h"

#include <sstream>

namespace aurora::netlist {

std::string DspfGenerator::generateDspf(const std::string& designName,
                                         const drc_lvs::ParasiticResult& p) const {
  std::ostringstream o;
  o << "*|DSPF 1.0\n";
  o << "*|DESIGN \"" << designName << "\"\n";
  o << "*|DATE \"2026-05-13\"\n";
  o << "*|VENDOR \"aurora-eda\"\n";
  std::size_t idx = 1;
  for (const auto& cap : p.caps) {
    o << "C" << idx << " " << cap.layer1 << " " << cap.layer2
      << " " << cap.capacitance << "f\n";
    ++idx;
  }
  for (const auto& r : p.resistors) {
    o << "R" << idx << " " << r.layer << " GND " << r.resistance << "\n";
    ++idx;
  }
  return o.str();
}

std::string DspfGenerator::generateRspf(const std::string& designName,
                                         const drc_lvs::ParasiticResult& p) const {
  std::ostringstream o;
  o << "*|RSPF 1.0\n";
  o << "*|DESIGN \"" << designName << "\"\n";
  for (const auto& r : p.resistors) {
    o << "R " << r.layer << " " << r.resistance << " " << r.length << "n\n";
  }
  return o.str();
}

std::string DspfGenerator::generateSdf(const std::string& designName,
                                        const drc_lvs::ParasiticResult& p) const {
  std::ostringstream o;
  o << "(DELAYFILE\n";
  o << "  (SDFVERSION \"3.0\")\n";
  o << "  (DESIGN \"" << designName << "\")\n";
  o << "  (VENDOR \"aurora-eda\")\n";
  std::size_t idx = 1;
  for (const auto& r : p.resistors) {
    double delay = r.resistance * 1e-12 * 1e9;  // very rough RC × 1pF in ns
    o << "  (CELL (CELLTYPE \"WIRE_" << idx << "\") "
      << "(DELAY (ABSOLUTE (IOPATH A Y (" << delay << "))))\n  )\n";
    ++idx;
  }
  o << ")\n";
  return o.str();
}

}  // namespace aurora::netlist
