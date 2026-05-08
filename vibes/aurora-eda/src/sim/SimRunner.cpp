#include "sim/SimRunner.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"
#include "netlist/NetlistGenerator.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#if defined(_WIN32)
#  define AURORA_POPEN  _popen
#  define AURORA_PCLOSE _pclose
#else
#  define AURORA_POPEN  popen
#  define AURORA_PCLOSE pclose
#endif

namespace aurora::sim {

SimRunner::SimRunner(std::filesystem::path workdir)
    : workdir_(std::move(workdir)) {}

void SimRunner::setSimulatorPath(std::filesystem::path path) {
  simulatorPath_ = std::move(path);
}

bool SimRunner::writeSpiceNetlist(const db::DbCellLib& lib, const db::DbCell& cell,
                                   const db::DbView& view) {
  std::error_code ec;
  std::filesystem::create_directories(workdir_, ec);
  if (ec) {
    lastError_ = "Failed to create workdir: " + ec.message();
    return false;
  }

  netlistPath_ = workdir_ / (cell.name() + ".spice");

  netlist::NetlistGenerator gen;
  const std::string spice = gen.generateSpice(lib, cell, view);

  std::ofstream ofs(netlistPath_);
  if (!ofs) {
    lastError_ = "Cannot open netlist file for writing: " + netlistPath_.string();
    return false;
  }
  ofs << spice;
  if (!ofs) {
    lastError_ = "Write failure for netlist file";
    return false;
  }
  return true;
}

std::string SimRunner::captureCommand(const std::string& cmd) {
  std::string output;
  FILE* pipe = AURORA_POPEN(cmd.c_str(), "r");
  if (!pipe) return {};
  char buf[512];
  while (std::fgets(buf, sizeof(buf), pipe) != nullptr)
    output += buf;
  AURORA_PCLOSE(pipe);
  return output;
}

SimResult SimRunner::run(std::string_view extraCommands) {
  SimResult result;

  if (netlistPath_.empty()) {
    result.errorMessage = "No netlist written; call writeSpiceNetlist() first";
    return result;
  }

  // Append a .control block so ngspice executes in batch mode
  const auto ctrlPath = workdir_ / "aurora_ctrl.sp";
  {
    std::ofstream ofs(ctrlPath);
    ofs << ".control\n";
    if (!extraCommands.empty()) ofs << extraCommands << "\n";
    ofs << "run\n";
    ofs << "print all\n";
    ofs << ".endc\n";
  }

  // Compose: ngspice -b -o <log> <netlist>
  const auto logPath = workdir_ / "aurora_sim.log";
  std::ostringstream cmd;
#if defined(_WIN32)
  cmd << '"' << simulatorPath_.string() << '"';
#else
  cmd << simulatorPath_.string();
#endif
  cmd << " -b -o \"" << logPath.string() << "\" \"" << netlistPath_.string() << "\" 2>&1";

  // Launch simulator — detect failure immediately so we can set errorMessage
  {
    FILE* pipe = AURORA_POPEN(cmd.str().c_str(), "r");
    if (!pipe) {
      result.errorMessage = "Failed to launch simulator. Check the simulator path: " +
                            simulatorPath_.string();
      return result;
    }
    char buf[512];
    while (std::fgets(buf, sizeof(buf), pipe) != nullptr)
      result.rawOutput += buf;
    AURORA_PCLOSE(pipe);
  }

  // Merge log file into rawOutput
  if (std::ifstream lf(logPath); lf) {
    std::ostringstream ss;
    ss << lf.rdbuf();
    if (!result.rawOutput.empty()) result.rawOutput += '\n';
    result.rawOutput += ss.str();
  }

  // Simple operating-point parser: lines matching "<name> = <value>"
  std::istringstream ss(result.rawOutput);
  std::string line;
  while (std::getline(ss, line)) {
    const auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    auto name = line.substr(0, eq);
    while (!name.empty() && name.front() == ' ') name.erase(0, 1);
    while (!name.empty() && name.back()  == ' ') name.pop_back();
    if (name.empty()) continue;
    try {
      result.dcOperatingPoint[name] = std::stod(line.substr(eq + 1));
    } catch (...) {}
  }

  result.success = result.errorMessage.empty();
  return result;
}

const std::filesystem::path& SimRunner::workdir()     const { return workdir_; }
const std::filesystem::path& SimRunner::netlistPath() const { return netlistPath_; }
const std::string&           SimRunner::lastError()   const { return lastError_; }

}  // namespace aurora::sim
