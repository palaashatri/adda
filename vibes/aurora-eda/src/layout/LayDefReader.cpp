#include "layout/LayDefReader.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbLayer.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbTypes.h"
#include "db/DbView.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace aurora::layout {

static std::string trimmed(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), s.end());
  return s;
}

bool LayDefReader::read(db::DbCellLib& lib, const std::filesystem::path& path) const {
  std::ifstream f(path);
  if (!f) return false;

  std::string line;
  std::string designName = "imported";
  db::DbCell* topCell = nullptr;
  db::DbView* topView = nullptr;

  enum class Section { None, Design, Components, Nets };
  Section section = Section::None;

  // First pass: find design name and create cell
  std::ifstream f2(path);
  while (std::getline(f2, line)) {
    line = trimmed(line);
    if (line.find("DESIGN ") == 0) {
      auto rest = line.substr(7);
      auto semi = rest.find(';');
      if (semi != std::string::npos) rest = rest.substr(0, semi);
      designName = trimmed(rest);
      break;
    }
  }

  if (designName.empty()) {
    designName = path.stem().string();
  }

  // Create top-level cell
  auto& cell = lib.createCell(designName);
  auto& view = cell.createView(db::DbViewType::Layout);
  topCell = &cell;
  topView = &view;

  // Ensure a default layer exists
  db::DbId defaultLayer = db::kInvalidId;
  for (const auto lid : lib.layerIds()) {
    defaultLayer = lid;
    break;
  }
  if (defaultLayer == db::kInvalidId) {
    auto& lay = lib.createLayer("def_import", "drawing");
    lay.setColor("#c0c0c0");
    defaultLayer = lay.id();
  }

  // Component instances: name → masterName mapping
  std::map<std::string, std::pair<std::string, db::DbTransform>> components;

  std::ifstream f3(path);
  while (std::getline(f3, line)) {
    line = trimmed(line);

    if (line.find("COMPONENTS") == 0) {
      section = Section::Components;
      continue;
    }
    if (line.find("NETS") == 0) {
      section = Section::Nets;
      continue;
    }
    if (line.find("END COMPONENTS") == 0 || line.find("END NETS") == 0 ||
        line.find("END DESIGN") == 0) {
      section = Section::None;
      continue;
    }

    if (section == Section::Components) {
      // Format: - inst_name cell_name + PLACED ( x y ) N ;
      if (line.size() < 3 || line[0] != '-' || line[1] != ' ') continue;
      auto rest = line.substr(2);
      // Find first space for instance name, then master name
      auto sp1 = rest.find(' ');
      if (sp1 == std::string::npos) continue;
      std::string instName = rest.substr(0, sp1);
      rest = trimmed(rest.substr(sp1 + 1));

      // Read until + or ;
      auto plusPos = rest.find(" +");
      std::string masterName;
      if (plusPos != std::string::npos) {
        masterName = trimmed(rest.substr(0, plusPos));
        rest = rest.substr(plusPos);
      } else {
        auto semiPos = rest.find(';');
        if (semiPos != std::string::npos) masterName = trimmed(rest.substr(0, semiPos));
        else masterName = trimmed(rest);
      }

      // Parse PLACED ( x y )
      db::DbTransform xform;
      auto placedPos = rest.find("PLACED");
      if (placedPos != std::string::npos) {
        auto parenOpen = rest.find('(', placedPos);
        auto parenClose = rest.find(')', parenOpen);
        if (parenOpen != std::string::npos && parenClose != std::string::npos) {
          std::istringstream coord(rest.substr(parenOpen + 1, parenClose - parenOpen - 1));
          double x = 0, y = 0;
          coord >> x >> y;
          xform.dx = static_cast<geom::DbUnit>(x * 1000); // convert µm to nm
          xform.dy = static_cast<geom::DbUnit>(y * 1000);
        }
      }

      components[instName] = {masterName, xform};
    }

    if (section == Section::Nets) {
      // Format: netName ( PIN pinName ) ( instName pinName ) ;
      // or: netName ( instName pinName ) ( instName pinName ) ;
      auto semiPos = line.find(" ;");
      if (semiPos == std::string::npos) semiPos = line.find(';');
      if (semiPos == std::string::npos) continue;

      std::string netSection = trimmed(line.substr(0, semiPos));

      // Parse net name (first token before first space or parenthesis)
      auto firstParen = netSection.find('(');
      std::string netName;
      if (firstParen != std::string::npos) {
        netName = trimmed(netSection.substr(0, firstParen));
      } else {
        continue;
      }
      if (netName.empty()) continue;

      // Create net
      auto& net = view.createNet(netName);

      // Parse pin connections
      size_t pos = firstParen;
      int pinCount = 0;
      while (pos < netSection.size()) {
        auto openP = netSection.find('(', pos);
        if (openP == std::string::npos) break;
        auto closeP = netSection.find(')', openP);
        if (closeP == std::string::npos) break;

        std::string inside = trimmed(netSection.substr(openP + 1, closeP - openP - 1));
        pos = closeP + 1;

        // inside is either "PIN pinName" or "instName pinName"
        if (inside.find("PIN ") == 0) {
          // Top-level port pin
          std::string pinName = trimmed(inside.substr(4));
          if (!pinName.empty()) {
            (void)view.createPin(pinName, db::DbPinDirection::Passive, net.id());
            ++pinCount;
          }
        } else {
          // Instance pin: "instName pinName"
          auto sp = inside.find(' ');
          if (sp != std::string::npos) {
            std::string instN = inside.substr(0, sp);
            std::string pinN = trimmed(inside.substr(sp + 1));
            auto it = components.find(instN);
            if (it != components.end()) {
              // Ensure the instance exists
              auto* existing = view.findInstanceByName(instN);
              db::DbId instId;
              if (existing) {
                instId = existing->id();
              } else {
                // Create instance if needed (should already exist from COMPONENTS)
                auto& newInst = view.createInstance(instN, db::kInvalidId, it->second.second);
                newInst.setParameter("DEF_MASTER", it->second.first);
                instId = newInst.id();
              }
              (void)view.createPin(pinN, db::DbPinDirection::Passive, net.id(), instId);
              ++pinCount;
            }
          }
        }
      }

      if (pinCount == 0) {
        // Remove empty net
        view.removeNet(net.id());
      }
    }
  }

  // Second pass: create instances from components
  for (const auto& [instName, data] : components) {
    const auto& [masterName, xform] = data;
    // Check if already created
    if (view.findInstanceByName(instName)) continue;
    auto& inst = view.createInstance(instName, db::kInvalidId, xform);
    inst.setParameter("DEF_MASTER", masterName);
  }

  return true;
}

}  // namespace aurora::layout
