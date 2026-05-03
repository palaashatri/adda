#pragma once

#include <cstdint>
#include <string_view>

namespace aurora::db {

using DbId = std::uint64_t;
inline constexpr DbId kInvalidId = 0;

enum class DbViewType {
  Schematic,
  Symbol,
  Layout,
  Abstract,
  Netlist,
};

enum class DbPinDirection {
  Input,
  Output,
  InOut,
  Passive,
  Unknown,
};

enum class DbShapeKind {
  Rect,
  Polygon,
  Path,
  Text,
};

[[nodiscard]] constexpr std::string_view toString(DbViewType type) {
  switch (type) {
    case DbViewType::Schematic:
      return "schematic";
    case DbViewType::Symbol:
      return "symbol";
    case DbViewType::Layout:
      return "layout";
    case DbViewType::Abstract:
      return "abstract";
    case DbViewType::Netlist:
      return "netlist";
  }
  return "unknown";
}

[[nodiscard]] constexpr std::string_view toString(DbPinDirection direction) {
  switch (direction) {
    case DbPinDirection::Input:
      return "input";
    case DbPinDirection::Output:
      return "output";
    case DbPinDirection::InOut:
      return "inout";
    case DbPinDirection::Passive:
      return "passive";
    case DbPinDirection::Unknown:
      return "unknown";
  }
  return "unknown";
}

}  // namespace aurora::db
