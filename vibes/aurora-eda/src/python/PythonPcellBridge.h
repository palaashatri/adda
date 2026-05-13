#pragma once

#include "db/DbTypes.h"
#include "db/DbView.h"
#include "pdk/PcellRegistry.h"

#include <string>

namespace aurora::python {

// A26 — Evaluate Python PCells from the C++ side. Uses pybind11 to import
// Python modules and call their generate_layout() method, producing shapes
// in a DbView.
//
// Example Python PCell:
//   class MyCell(PcellBase):
//     @classmethod
//     def generate_layout(cls, view, tech, params):
//       view.create_rect(layer_id, Box(0,0,1000,2000))

class PythonPcellEvaluator : public pdk::PcellEvaluatorBase {
 public:
  // Register evaluator: always available to try calling.
  PythonPcellEvaluator(const std::string& modulePath,
                       const std::string& className);

  // Evaluate the PCell. Returns true on success.
  bool evaluate(db::DbView& view, const std::map<std::string, std::string>& params) const override;

  [[nodiscard]] std::string name() const override { return name_; }

 private:
  std::string modulePath_;
  std::string className_;
  std::string name_;
};

}  // namespace aurora::python
