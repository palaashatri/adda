#include "netlist/VerilogImporter.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbView.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace aurora::netlist {

namespace {

std::string stripComments(const std::string& src) {
  std::string out;
  out.reserve(src.size());
  size_t i = 0;
  while (i < src.size()) {
    if (i + 1 < src.size() && src[i] == '/' && src[i+1] == '/') {
      while (i < src.size() && src[i] != '\n') ++i;
    } else if (i + 1 < src.size() && src[i] == '/' && src[i+1] == '*') {
      i += 2;
      while (i + 1 < src.size() && !(src[i] == '*' && src[i+1] == '/')) ++i;
      i += 2;
    } else {
      out += src[i++];
    }
  }
  return out;
}

std::string trim(const std::string& s) {
  auto b = s.find_first_not_of(" \t\r\n");
  if (b == std::string::npos) return "";
  auto e = s.find_last_not_of(" \t\r\n");
  return s.substr(b, e - b + 1);
}

std::vector<std::string> splitCsv(const std::string& s) {
  std::vector<std::string> out;
  std::string cur;
  int depth = 0;
  for (char c : s) {
    if (c == '(' || c == '{') depth++;
    else if (c == ')' || c == '}') depth--;
    if (c == ',' && depth == 0) { out.push_back(trim(cur)); cur.clear(); }
    else cur += c;
  }
  if (!trim(cur).empty()) out.push_back(trim(cur));
  return out;
}

}  // namespace

bool VerilogImporter::importFile(db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ifstream in(path);
  if (!in) return false;
  std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  src = stripComments(src);

  std::regex moduleRe(R"(module\s+(\w+)\s*\(([^)]*)\)\s*;([\s\S]*?)endmodule)");
  for (std::sregex_iterator it(src.begin(), src.end(), moduleRe), end; it != end; ++it) {
    const std::string modName = (*it)[1];
    const std::string portList = (*it)[2];
    const std::string body = (*it)[3];

    auto* cell = lib.findCell(modName);
    if (!cell) cell = &lib.createCell(modName);
    auto& view = cell->createView(db::DbViewType::Schematic);

    // Ports.
    for (const auto& p : splitCsv(portList)) {
      if (p.empty()) continue;
      auto& net = view.createNet(p);
      (void)view.createPin(p, db::DbPinDirection::InOut, net.id());
    }
    // Direction declarations: input/output/inout ports
    std::regex dirRe(R"((input|output|inout)\s+([^;]+);)");
    for (std::sregex_iterator d(body.begin(), body.end(), dirRe), e2; d != e2; ++d) {
      const std::string dir = (*d)[1];
      const std::string names = (*d)[2];
      db::DbPinDirection pdir = db::DbPinDirection::InOut;
      if (dir == "input") pdir = db::DbPinDirection::Input;
      else if (dir == "output") pdir = db::DbPinDirection::Output;
      for (const auto& n : splitCsv(names)) {
        auto trimmed = trim(n);
        if (trimmed.empty()) continue;
        for (auto pid : view.pinIds()) {
          auto* pp = view.findPin(pid);
          if (pp && pp->name() == trimmed) { pp->setDirection(pdir); break; }
        }
      }
    }

    // wire declarations
    std::regex wireRe(R"(wire\s+([^;]+);)");
    for (std::sregex_iterator w(body.begin(), body.end(), wireRe), e3; w != e3; ++w) {
      for (const auto& n : splitCsv((*w)[1])) {
        auto trimmed = trim(n);
        if (!trimmed.empty() && !view.findNetByName(trimmed)) {
          (void)view.createNet(trimmed);
        }
      }
    }

    // Instances: <master> <inst_name> ( .port(net), ... );
    std::regex instRe(R"((\w+)\s+(\w+)\s*\(([\s\S]*?)\)\s*;)");
    for (std::sregex_iterator iIt(body.begin(), body.end(), instRe), e4; iIt != e4; ++iIt) {
      const std::string master = (*iIt)[1];
      const std::string instName = (*iIt)[2];
      if (master == "input" || master == "output" || master == "inout" || master == "wire") continue;

      auto* masterCell = lib.findCell(master);
      if (!masterCell) masterCell = &lib.createCell(master);

      auto& inst = view.createInstance(instName, masterCell->id());
      // Connections .port(net)
      const std::string conns = (*iIt)[3];
      std::regex pinRe(R"(\.(\w+)\s*\(\s*([^)]*)\s*\))");
      for (std::sregex_iterator c(conns.begin(), conns.end(), pinRe), e5; c != e5; ++c) {
        const std::string portName = (*c)[1];
        const std::string netName = trim((*c)[2]);
        auto* net = view.findNetByName(netName);
        if (!net) net = &view.createNet(netName);
        (void)view.createPin(portName, db::DbPinDirection::InOut, net->id(), inst.id());
      }
    }
  }
  return true;
}

}  // namespace aurora::netlist
