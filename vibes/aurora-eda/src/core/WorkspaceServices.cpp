#include "core/WorkspaceServices.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbView.h"

#include <cctype>
#include <chrono>
#include <fstream>
#include <regex>
#include <sstream>

namespace aurora::core {

namespace fs = std::filesystem;

void WorkspaceLayout::setDock(DockPosition d) {
  for (auto& x : docks_) {
    if (x.name == d.name) { x = std::move(d); return; }
  }
  docks_.push_back(std::move(d));
}

bool WorkspaceLayout::save(const fs::path& path) const {
  std::ofstream o(path);
  if (!o) return false;
  o << "# aurora-eda workspace layout\n";
  for (const auto& d : docks_) {
    o << "DOCK " << d.name << " " << d.area << " " << d.width << " " << d.height
      << " " << (d.visible ? 1 : 0) << "\n";
  }
  return true;
}

bool WorkspaceLayout::load(const fs::path& path) {
  std::ifstream in(path);
  if (!in) return false;
  docks_.clear();
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream s(line);
    std::string tok; s >> tok;
    if (tok == "DOCK") {
      DockPosition d;
      int vis = 1;
      s >> d.name >> d.area >> d.width >> d.height >> vis;
      d.visible = (vis != 0);
      docks_.push_back(d);
    }
  }
  return true;
}

Theme ThemeManager::dark() {
  Theme t;
  t.name = "dark";
  t.background = "#1e1e1e";
  t.foreground = "#dcdcdc";
  t.accent = "#3a8ee6";
  t.grid = "#404040";
  return t;
}

Theme ThemeManager::light() {
  Theme t;
  t.name = "light";
  t.background = "#fafafa";
  t.foreground = "#202020";
  t.accent = "#1976d2";
  t.grid = "#cccccc";
  return t;
}

std::vector<SearchHit> DesignSearch::findNet(const db::DbCellLib& lib, const std::string& pattern) {
  std::vector<SearchHit> hits;
  std::regex re(pattern);
  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    for (auto vid : cell->viewIds()) {
      const auto* v = cell->findViewById(vid);
      if (!v) continue;
      for (auto nid : v->netIds()) {
        const auto* n = v->findNet(nid);
        if (n && std::regex_search(n->name(), re)) {
          hits.push_back({cell->name(), "net", n->name(), vid});
        }
      }
    }
  }
  return hits;
}

std::vector<SearchHit> DesignSearch::findInstance(const db::DbCellLib& lib,
                                                   const std::string& pattern) {
  std::vector<SearchHit> hits;
  std::regex re(pattern);
  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    for (auto vid : cell->viewIds()) {
      const auto* v = cell->findViewById(vid);
      if (!v) continue;
      for (auto iid : v->instanceIds()) {
        const auto* inst = v->findInstance(iid);
        if (inst && std::regex_search(inst->name(), re)) {
          hits.push_back({cell->name(), "instance", inst->name(), vid});
        }
      }
    }
  }
  return hits;
}

std::size_t DesignSearch::replaceNetName(db::DbCellLib& lib,
                                          const std::string& from,
                                          const std::string& to) {
  std::size_t n = 0;
  for (auto cid : lib.cellIds()) {
    auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    for (auto vid : cell->viewIds()) {
      auto* v = cell->findViewById(vid);
      if (!v) continue;
      for (auto nid : v->netIds()) {
        auto* net = v->findNet(nid);
        if (net && net->name() == from) { net->setName(to); ++n; }
      }
    }
  }
  return n;
}

void TechRuleEditor::setRule(TechRuleEntry e) {
  for (auto& r : rules_) {
    if (r.layer == e.layer && r.ruleKind == e.ruleKind) { r = std::move(e); return; }
  }
  rules_.push_back(std::move(e));
}

bool TechRuleEditor::exportJson(const fs::path& path) const {
  std::ofstream o(path);
  if (!o) return false;
  o << "{\n  \"rules\": [\n";
  for (size_t i = 0; i < rules_.size(); ++i) {
    const auto& r = rules_[i];
    o << "    { \"layer\": \"" << r.layer << "\","
      << " \"kind\": \"" << r.ruleKind << "\","
      << " \"value\": " << r.value << ","
      << " \"unit\": \"" << r.unit << "\" }";
    if (i + 1 < rules_.size()) o << ",";
    o << "\n";
  }
  o << "  ]\n}\n";
  return true;
}

void HotkeyConfig::bind(const std::string& action, const std::string& key) {
  bindings_[action] = key;
}

std::string HotkeyConfig::keyFor(const std::string& action) const {
  auto it = bindings_.find(action);
  return it != bindings_.end() ? it->second : "";
}

bool HotkeyConfig::save(const fs::path& path) const {
  std::ofstream o(path);
  if (!o) return false;
  for (const auto& [a, k] : bindings_) o << a << " = " << k << "\n";
  return true;
}

bool HotkeyConfig::load(const fs::path& path) {
  std::ifstream in(path);
  if (!in) return false;
  std::string line;
  while (std::getline(in, line)) {
    auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string a = line.substr(0, eq);
    std::string k = line.substr(eq + 1);
    auto trim = [](std::string& s) {
      while (!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
      while (!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
    };
    trim(a); trim(k);
    if (!a.empty()) bindings_[a] = k;
  }
  return true;
}

void NotificationCenter::post(Notification n) {
  using namespace std::chrono;
  if (n.timestamp == 0) {
    n.timestamp = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  }
  items_.push_back(std::move(n));
}

}  // namespace aurora::core
