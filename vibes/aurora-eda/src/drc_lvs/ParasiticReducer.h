#pragma once

#include "drc_lvs/ParasiticExtractor.h"

namespace aurora::drc_lvs {

// E7 — Parasitic reduction: collapse extracted RC networks to compact models.
enum class ReductionModel {
  Pi,   // single R between two terminals + cap at each end
  T,    // two R in series with a cap to ground at center
  Lumped,
};

struct ReducedNet {
  std::string netName;
  ReductionModel model{ReductionModel::Pi};
  double r1{0.0};  // ohms
  double r2{0.0};  // ohms (T-model)
  double c1{0.0};  // fF
  double c2{0.0};  // fF
  double cMid{0.0};// fF (T-model midpoint)
};

struct ReducedParasitics {
  std::vector<ReducedNet> nets;
};

class ParasiticReducer {
 public:
  [[nodiscard]] static ReducedParasitics reduce(const ParasiticResult& full,
                                                ReductionModel model = ReductionModel::Pi);
};

}  // namespace aurora::drc_lvs
