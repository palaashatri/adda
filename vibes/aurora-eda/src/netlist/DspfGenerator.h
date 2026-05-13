#pragma once

#include <string>

namespace aurora::drc_lvs {
struct ParasiticResult;
}

namespace aurora::netlist {

// G11 — DSPF / RSPF / SDF parasitic export.
class DspfGenerator {
 public:
  [[nodiscard]] std::string generateDspf(const std::string& designName,
                                          const drc_lvs::ParasiticResult& parasitics) const;
  [[nodiscard]] std::string generateRspf(const std::string& designName,
                                          const drc_lvs::ParasiticResult& parasitics) const;
  [[nodiscard]] std::string generateSdf(const std::string& designName,
                                         const drc_lvs::ParasiticResult& parasitics) const;
};

}  // namespace aurora::netlist
