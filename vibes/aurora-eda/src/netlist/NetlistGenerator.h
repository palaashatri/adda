#pragma once

#include "db/DbCell.h"
#include "db/DbView.h"

#include <string>

namespace aurora::netlist {

class NetlistGenerator {
 public:
  [[nodiscard]] std::string generateSpice(const db::DbCell& cell, const db::DbView& view) const;
};

}  // namespace aurora::netlist
