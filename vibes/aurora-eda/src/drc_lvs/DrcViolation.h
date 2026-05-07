#pragma once

#include "geom/GeomBox.h"

#include <string>

namespace aurora::drc_lvs {

enum class DrcViolationType {
  MinWidth,
  MinSpacing,
  Enclosure,
  NonManhattan,
};

struct DrcViolation {
  DrcViolationType type;
  std::string      layerName;
  std::string      message;
  geom::GeomBox    location;
};

}  // namespace aurora::drc_lvs
