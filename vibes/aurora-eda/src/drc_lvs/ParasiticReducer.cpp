#include "drc_lvs/ParasiticReducer.h"

#include <map>

namespace aurora::drc_lvs {

ReducedParasitics ParasiticReducer::reduce(const ParasiticResult& full, ReductionModel model) {
  ReducedParasitics out;

  // Group caps/resistors per layer pair as a "net stand-in" since we lack net IDs here.
  // Sum R per layer, distribute C equally to the two terminals (Pi) or split for T.
  std::map<std::string, ReducedNet> byLayer;
  for (const auto& r : full.resistors) {
    auto& rn = byLayer[r.layer];
    rn.netName = r.layer;
    rn.model = model;
    if (model == ReductionModel::T) {
      rn.r1 += r.resistance * 0.5;
      rn.r2 += r.resistance * 0.5;
    } else {
      rn.r1 += r.resistance;
    }
  }
  for (const auto& c : full.caps) {
    auto& rn = byLayer[c.layer1];
    rn.netName = c.layer1;
    rn.model = model;
    if (model == ReductionModel::T) {
      rn.cMid += c.capacitance;
    } else {
      rn.c1 += c.capacitance * 0.5;
      rn.c2 += c.capacitance * 0.5;
    }
  }
  for (auto& [_, rn] : byLayer) out.nets.push_back(rn);
  return out;
}

}  // namespace aurora::drc_lvs
