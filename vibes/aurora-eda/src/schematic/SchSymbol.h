#pragma once

#include "db/DbTypes.h"

#include <string>
#include <vector>

namespace aurora::schematic {

class SchSymbol {
 public:
  explicit SchSymbol(std::string name = {});

  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] const std::vector<db::DbId>& pinIds() const;
  void addPin(db::DbId pinId);

 private:
  std::string name_;
  std::vector<db::DbId> pinIds_;
};

}  // namespace aurora::schematic
