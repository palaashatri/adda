#include "sim/SimRunner.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"
#include "netlist/NetlistGenerator.h"

#include <cmath>
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

  // Parse waveform tables from SPICE output (format: header line then data rows)
  {
    std::istringstream ss2(result.rawOutput);
    std::string line2;
    bool inTable = false;
    std::vector<std::string> headers;
    std::vector<std::vector<double>> columns;

    while (std::getline(ss2, line2)) {
      // Detect SPICE table header: starts with "Index" followed by whitespace
      if (line2.find("Index") == 0 && line2.find('\t') == std::string::npos) {
        std::istringstream hdr(line2);
        std::string tok;
        headers.clear();
        columns.clear();
        while (hdr >> tok) {
          if (tok != "Index") {
            headers.push_back(tok);
            columns.emplace_back();
          }
        }
        inTable = !headers.empty();
        continue;
      }

      if (!inTable) continue;

      // Parse data row
      std::istringstream data(line2);
      double idx = 0.0;
      data >> idx;
      if (data.fail()) { inTable = false; continue; } // end of table

      for (std::size_t ci = 0; ci < columns.size(); ++ci) {
        double val = 0.0;
        data >> val;
        if (data.fail()) break;
        columns[ci].push_back(val);
      }
    }

    // Build waveforms from parsed columns
    // The x-axis is always the first column (e.g., time, sweep variable)
    if (columns.size() >= 2) {
      std::vector<double> xAxis = std::move(columns[0]);
      for (std::size_t ci = 1; ci < columns.size(); ++ci) {
        if (columns[ci].size() == xAxis.size()) {
          result.waveforms.push_back(
              {std::move(headers[ci]), xAxis, std::move(columns[ci])});
        }
      }
    }
  }

  // Simple operating-point parser: lines matching "<name> = <value>"
  {
    std::istringstream ss3(result.rawOutput);
    std::string line3;
    while (std::getline(ss3, line3)) {
      const auto eq = line3.find('=');
      if (eq == std::string::npos) continue;
      auto name = line3.substr(0, eq);
      while (!name.empty() && name.front() == ' ') name.erase(0, 1);
      while (!name.empty() && name.back()  == ' ') name.pop_back();
      if (name.empty()) continue;
      try {
        result.dcOperatingPoint[name] = std::stod(line3.substr(eq + 1));
      } catch (...) {}
    }
  }

  result.success = result.errorMessage.empty();
  return result;
}

std::vector<SimResult> SimRunner::runSweep(const SweepParam& param,
                                            const std::string& extraCommands) {
  std::vector<SimResult> results;
  for (int step = 0; step <= param.steps; ++step) {
    double value;
    if (param.logScale) {
      value = param.start * std::pow(param.stop / param.start,
                                     static_cast<double>(step) / param.steps);
    } else {
      value = param.start + (param.stop - param.start) * step / param.steps;
    }

    // Replace $PARAM in extra commands
    std::string cmd = extraCommands;
    auto pos = cmd.find("$PARAM");
    while (pos != std::string::npos) {
      cmd.replace(pos, 6, std::to_string(value));
      pos = cmd.find("$PARAM", pos + 1);
    }

    // Also replace in .param statement if present
    std::string paramsCmd = cmd;
    auto ppos = paramsCmd.find(".param " + param.name);
    if (ppos != std::string::npos) {
      // Replace the value after = in .param <name> = <value>
      auto eqPos = paramsCmd.find('=', ppos);
      if (eqPos != std::string::npos) {
        auto endPos = paramsCmd.find_first_of(" \t\n", eqPos + 1);
        if (endPos == std::string::npos) endPos = paramsCmd.size();
        paramsCmd.replace(eqPos + 1, endPos - eqPos - 1, " " + std::to_string(value));
      }
    }

    auto r = run(paramsCmd);
    r.rawOutput = std::to_string(value) + "\n" + r.rawOutput; // tag with value
    results.push_back(std::move(r));
  }
  return results;
}

const std::filesystem::path& SimRunner::workdir()     const { return workdir_; }
const std::filesystem::path& SimRunner::netlistPath() const { return netlistPath_; }
const std::string&           SimRunner::lastError()   const { return lastError_; }

}  // namespace aurora::sim
