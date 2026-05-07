#pragma once

#include "sim/SimResult.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace aurora::db {
class DbCell;
class DbView;
class DbCellLib;
}  // namespace aurora::db

namespace aurora::sim {

// Runs a SPICE-compatible simulator (ngspice by default) on a cell view.
// Call writeSpiceNetlist() first, then run().
class SimRunner {
 public:
  explicit SimRunner(std::filesystem::path workdir);

  void setSimulatorPath(std::filesystem::path path);

  // Write a SPICE netlist derived from the given cell/view to workdir.
  // Returns false and sets lastError() on failure.
  [[nodiscard]] bool writeSpiceNetlist(const db::DbCellLib& lib, const db::DbCell& cell,
                                       const db::DbView& view);

  // Run the simulator. extraCommands is appended to the .control block.
  [[nodiscard]] SimResult run(std::string_view extraCommands = "");

  [[nodiscard]] const std::filesystem::path& workdir() const;
  [[nodiscard]] const std::filesystem::path& netlistPath() const;
  [[nodiscard]] const std::string& lastError() const;

 private:
  [[nodiscard]] static std::string captureCommand(const std::string& cmd);

  std::filesystem::path workdir_;
  std::filesystem::path simulatorPath_{"ngspice"};
  std::filesystem::path netlistPath_;
  std::string lastError_;
};

}  // namespace aurora::sim
