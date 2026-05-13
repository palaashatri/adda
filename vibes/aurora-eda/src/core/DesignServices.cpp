#include "core/DesignServices.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbView.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <set>

namespace aurora::core {

namespace fs = std::filesystem;

HierarchyNode HierarchyBrowser::build(const db::DbCellLib& lib, const std::string& topCell) {
  HierarchyNode root;
  root.cellName = topCell;
  const auto* cell = lib.findCell(topCell);
  if (!cell) return root;

  std::set<std::string> visited;
  std::function<void(HierarchyNode&, const db::DbCell&)> recurse =
      [&](HierarchyNode& node, const db::DbCell& c) {
    if (!visited.insert(c.name()).second) return;
    for (auto vid : c.viewIds()) {
      const auto* v = c.findViewById(vid);
      if (!v) continue;
      for (auto iid : v->instanceIds()) {
        const auto* inst = v->findInstance(iid);
        if (!inst) continue;
        const auto* m = lib.findCellById(inst->masterCellId());
        if (!m) continue;
        HierarchyNode child;
        child.cellName = m->name();
        child.usedBy.insert(c.name());
        recurse(child, *m);
        node.children.push_back(std::move(child));
      }
    }
  };
  recurse(root, *cell);
  return root;
}

std::vector<std::string> HierarchyBrowser::findUsage(const db::DbCellLib& lib,
                                                      const std::string& cellName) {
  std::vector<std::string> users;
  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell || cell->name() == cellName) continue;
    bool found = false;
    for (auto vid : cell->viewIds()) {
      const auto* v = cell->findViewById(vid);
      if (!v) continue;
      for (auto iid : v->instanceIds()) {
        const auto* inst = v->findInstance(iid);
        if (!inst) continue;
        const auto* m = lib.findCellById(inst->masterCellId());
        if (m && m->name() == cellName) { found = true; break; }
      }
      if (found) break;
    }
    if (found) users.push_back(cell->name());
  }
  return users;
}

bool ProjectArchiver::archive(const fs::path& projectDir, const fs::path& outFile) {
  if (!fs::is_directory(projectDir)) return false;
  std::ofstream o(outFile, std::ios::binary);
  if (!o) return false;
  o << "AURORA-AR-1\n";
  for (const auto& ent : fs::recursive_directory_iterator(projectDir)) {
    if (!ent.is_regular_file()) continue;
    auto rel = fs::relative(ent.path(), projectDir);
    std::ifstream in(ent.path(), std::ios::binary);
    if (!in) continue;
    in.seekg(0, std::ios::end);
    auto sz = static_cast<long long>(in.tellg());
    in.seekg(0);
    o << "F " << rel.generic_string() << " " << sz << "\n";
    o << in.rdbuf();
    o << "\n";
  }
  o << "EOF\n";
  return true;
}

bool ProjectArchiver::extract(const fs::path& archiveFile, const fs::path& destDir) {
  std::ifstream in(archiveFile, std::ios::binary);
  if (!in) return false;
  std::string header;
  std::getline(in, header);
  if (header.find("AURORA-AR-1") == std::string::npos) return false;
  fs::create_directories(destDir);
  std::string line;
  while (std::getline(in, line)) {
    if (line == "EOF") break;
    if (line.substr(0, 2) != "F ") continue;
    auto rest = line.substr(2);
    auto sp = rest.rfind(' ');
    if (sp == std::string::npos) continue;
    std::string rel = rest.substr(0, sp);
    long long sz = std::stoll(rest.substr(sp + 1));
    fs::path full = destDir / rel;
    fs::create_directories(full.parent_path());
    std::ofstream o(full, std::ios::binary);
    if (sz > 0) {
      std::string buf(sz, '\0');
      in.read(buf.data(), sz);
      o.write(buf.data(), sz);
    }
    in.get();  // newline
  }
  return true;
}

DesignHealthReport DesignHealthDashboard::compile(const db::DbCellLib& lib) {
  DesignHealthReport r;
  for (auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    ++r.cellCount;
    for (auto vid : cell->viewIds()) {
      const auto* v = cell->findViewById(vid);
      if (!v) continue;
      r.shapeCount    += v->shapeIds().size();
      r.instanceCount += v->instanceIds().size();
      r.netCount      += v->netIds().size();
    }
  }
  r.cellsPerView["total"] = r.cellCount;
  return r;
}

}  // namespace aurora::core
