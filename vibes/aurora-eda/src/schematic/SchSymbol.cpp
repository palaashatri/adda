#include "schematic/SchSymbol.h"

#include <algorithm>
#include <utility>

namespace aurora::schematic {

SchSymbol::SchSymbol(std::string name) : name_(std::move(name)) {}

const std::string& SchSymbol::name() const {
  return name_;
}

const std::vector<db::DbId>& SchSymbol::pinIds() const {
  return pinIds_;
}

void SchSymbol::addPin(db::DbId pinId) {
  if (std::find(pinIds_.begin(), pinIds_.end(), pinId) == pinIds_.end()) {
    pinIds_.push_back(pinId);
  }
}

}  // namespace aurora::schematic
