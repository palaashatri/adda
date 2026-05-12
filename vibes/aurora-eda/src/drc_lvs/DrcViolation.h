#pragma once

#include "geom/GeomBox.h"

#include <string>

namespace aurora::drc_lvs {

enum class DrcViolationType {
  MinWidth,
  MinSpacing,
  Enclosure,
  NonManhattan,
  ERC,        // E10
  Antenna,    // E8
  Density,    // E9
};

struct DrcViolation {
  DrcViolationType type;
  std::string      layerName;
  std::string      message;
  geom::GeomBox    location;
};

}  // namespace aurora::drc_lvs
