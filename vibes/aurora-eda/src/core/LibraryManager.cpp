#include "core/LibraryManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace aurora::core {

void LibraryManager::attach(const std::string& name, const std::filesystem::path& path,
                             int priority) {
  LibraryEntry e;
  e.name = name;
  e.path = path;
  e.priority = priority;
  e.attached = true;
  libs_[name] = std::move(e);
}

void LibraryManager::detach(const std::string& name) {
  auto it = libs_.find(name);
  if (it != libs_.end()) it->second.attached = false;
}

LibraryEntry* LibraryManager::find(std::string_view name) {
  auto it = libs_.find(name);
  return it != libs_.end() ? &it->second : nullptr;
}

const LibraryEntry* LibraryManager::find(std::string_view name) const {
  auto it = libs_.find(name);
  return it != libs_.end() ? &it->second : nullptr;
}

std::vector<LibraryEntry> LibraryManager::attachedSorted() const {
  std::vector<LibraryEntry> out;
  for (const auto& [_, e] : libs_) if (e.attached) out.push_back(e);
  std::sort(out.begin(), out.end(),
            [](const LibraryEntry& a, const LibraryEntry& b) {
              return a.priority > b.priority;
            });
  return out;
}

void LibraryManager::addSearchPath(std::filesystem::path p) {
  searchPaths_.push_back(std::move(p));
}

const std::vector<std::filesystem::path>& LibraryManager::searchPaths() const {
  return searchPaths_;
}

void LibraryManager::setTechFile(const std::string& libName, const std::string& techPath) {
  auto it = libs_.find(libName);
  if (it != libs_.end()) it->second.techFile = techPath;
}

void LibraryManager::setRevision(const std::string& libName, std::string rev) {
  auto it = libs_.find(libName);
  if (it != libs_.end()) it->second.revision = std::move(rev);
}

std::string LibraryManager::diffSnapshots(const std::string& jsonA, const std::string& jsonB) {
  // Minimal line-by-line diff.
  std::istringstream sa(jsonA), sb(jsonB);
  std::vector<std::string> la, lb;
  std::string l;
  while (std::getline(sa, l)) la.push_back(l);
  while (std::getline(sb, l)) lb.push_back(l);

  std::ostringstream o;
  const std::size_t n = std::max(la.size(), lb.size());
  for (std::size_t i = 0; i < n; ++i) {
    const std::string& a = i < la.size() ? la[i] : "";
    const std::string& b = i < lb.size() ? lb[i] : "";
    if (a != b) {
      if (!a.empty()) o << "- " << a << "\n";
      if (!b.empty()) o << "+ " << b << "\n";
    }
  }
  return o.str();
}

bool LibraryManager::writeCdsLib(const std::filesystem::path& path) const {
  std::ofstream o(path);
  if (!o) return false;
  o << "# aurora-eda library map (cds.lib-style)\n";
  for (const auto& [name, e] : libs_) {
    o << "DEFINE " << name << " " << e.path.string()
      << "  ; prio=" << e.priority << ", attached=" << (e.attached ? 1 : 0);
    if (!e.techFile.empty()) o << ", tech=" << e.techFile;
    if (!e.revision.empty()) o << ", rev=" << e.revision;
    o << "\n";
  }
  for (const auto& p : searchPaths_) o << "SEARCH " << p.string() << "\n";
  return true;
}

bool LibraryManager::readCdsLib(const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in) return false;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream s(line);
    std::string tok; s >> tok;
    if (tok == "DEFINE") {
      std::string name, p;
      s >> name >> p;
      attach(name, p);
    } else if (tok == "SEARCH") {
      std::string p; s >> p;
      addSearchPath(p);
    }
  }
  return true;
}

}  // namespace aurora::core
