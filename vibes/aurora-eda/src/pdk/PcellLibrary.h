#pragma once

#include "pdk/PcellRegistry.h"

namespace aurora::pdk {

// F7-F11 — Built-in PCell library.
// Registers MOS (NMOS/PMOS), passives (R/C/L), BJT (NPN/PNP),
// diodes (PN, Schottky, ESD), and matching structures (common-centroid).
void registerPcellLibrary(PcellRegistry& registry);

// F5 — Stretch handles for parameterized resizing of PCells.
struct StretchHandle {
  std::string paramName;
  enum class Axis { X, Y } axis{Axis::X};
  long long anchorX{0};
  long long anchorY{0};
  double unitsPerNm{1.0};  // how many param-units per 1 nm of drag
};

[[nodiscard]] std::vector<StretchHandle> defaultStretchHandlesFor(std::string_view pcellName);

}  // namespace aurora::pdk
