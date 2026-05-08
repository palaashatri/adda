#include "netlist/SpiceImporter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace aurora::netlist {

namespace {

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return (char)std::tolower(c); });
  return s;
}

std::vector<std::string> split(const std::string& s) {
  std::vector<std::string> tokens;
  std::istringstream ss(s);
  std::string tok;
  while (ss >> tok) tokens.push_back(tok);
  return tokens;
}

// Join continuation lines (SPICE lines starting with '+')
std::vector<std::string> joinContinuation(const std::vector<std::string>& raw) {
  std::vector<std::string> lines;
  for (const auto& raw_line : raw) {
    if (raw_line.empty() || raw_line[0] == '*') continue;  // comment
    std::string stripped = raw_line;
    // Remove inline comments
    if (const auto pos = stripped.find(';'); pos != std::string::npos)
      stripped = stripped.substr(0, pos);
    if (stripped.empty()) continue;
    if (stripped[0] == '+' && !lines.empty())
      lines.back() += ' ' + stripped.substr(1);
    else
      lines.push_back(stripped);
  }
  return lines;
}

}  // namespace

bool SpiceImporter::importFile(const std::filesystem::path& path, db::DbCellLib& lib) {
  std::ifstream f(path);
  if (!f) {
    lastError_ = "Cannot open file: " + path.string();
    return false;
  }
  std::vector<std::string> raw;
  std::string line;
  while (std::getline(f, line)) raw.push_back(line);
  return parseLines(joinContinuation(raw), lib);
}

bool SpiceImporter::importString(const std::string& spice, db::DbCellLib& lib) {
  std::vector<std::string> raw;
  std::istringstream ss(spice);
  std::string line;
  while (std::getline(ss, line)) raw.push_back(line);
  return parseLines(joinContinuation(raw), lib);
}

bool SpiceImporter::parseLines(const std::vector<std::string>& lines, db::DbCellLib& lib) {
  db::DbCell* currentCell = nullptr;
  db::DbView* currentView = nullptr;

  for (const auto& line : lines) {
    const auto toks = split(line);
    if (toks.empty()) continue;
    const auto keyword = toLower(toks[0]);

    if (keyword == ".subckt") {
      // .subckt <name> <port1> <port2> ...
      if (toks.size() < 2) continue;
      const auto cellName = toks[1];
      currentCell = &lib.createCell(cellName);
      currentView = &currentCell->createView(db::DbViewType::Schematic);
      // Create port nets
      for (std::size_t i = 2; i < toks.size(); ++i) {
        if (toks[i][0] == '$') continue;  // skip SPICE options like $SUB
        currentView->createNet(toks[i]);
      }
    } else if (keyword == ".ends") {
      currentCell = nullptr;
      currentView = nullptr;
    } else if (currentView && !toks.empty()) {
      const char prefix = std::toupper((unsigned char)toks[0][0]);
      if (prefix == 'X' && toks.size() >= 3) {
        // X<name> <net1> ... <netN> <masterCell> [params]
        const auto instName  = toks[0];
        const auto masterName = toks[toks.size() - 1];  // last positional arg (before params)
        // Find master cell (may not exist yet, create a stub)
        db::DbId masterCellId = db::kInvalidId;
        for (const auto cid : lib.cellIds()) {
          if (const auto* c = lib.findCellById(cid))
            if (c->name() == masterName) { masterCellId = cid; break; }
        }
        if (masterCellId == db::kInvalidId) {
          auto& stub = lib.createCell(masterName);
          masterCellId = stub.id();
        }
        currentView->createInstance(instName, masterCellId);
        // Create nets for the connected nodes
        for (std::size_t i = 1; i + 1 < toks.size(); ++i) {
          const auto& netName = toks[i];
          if (netName.find('=') != std::string::npos) break;  // hit parameter section
          if (!currentView->findNetByName(netName))
            currentView->createNet(netName);
        }
      } else if ((prefix == 'R' || prefix == 'C' || prefix == 'L' || prefix == 'V' ||
                  prefix == 'I' || prefix == 'M') && toks.size() >= 4) {
        // Passive/source elements: <name> <n+> <n-> [other nodes] <value/model> [params]
        const auto& n1 = toks[1];
        const auto& n2 = toks[2];
        if (!currentView->findNetByName(n1)) currentView->createNet(n1);
        if (!currentView->findNetByName(n2)) currentView->createNet(n2);
      }
    }
  }
  return true;
}

}  // namespace aurora::netlist
