#pragma once

#include "sim/SimResult.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace aurora::db {
class DbCell;
class DbView;
class DbCellLib;
}  // namespace aurora::db

namespace aurora::sim {

struct SweepParam {
  std::string name;
  double start{0};
  double stop{1};
  int steps{10};
  bool logScale{false};
};

// Runs a SPICE-compatible simulator (ngspice by default) on a cell view.
// Call writeSpiceNetlist() first, then run().
class SimRunner {
 public:
  explicit SimRunner(std::filesystem::path workdir);

  void setSimulatorPath(std::filesystem::path path);

  [[nodiscard]] bool writeSpiceNetlist(const db::DbCellLib& lib, const db::DbCell& cell,
                                       const db::DbView& view);
  [[nodiscard]] SimResult run(std::string_view extraCommands = "");

  // Run a parametric sweep: replaces $PARAM in extraCommands with each sweep value.
  // The .param statement should be included in extraCommands.
  [[nodiscard]] std::vector<SimResult> runSweep(const SweepParam& param,
                                                const std::string& extraCommands);

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
