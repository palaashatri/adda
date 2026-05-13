#pragma once

#include "db/DbCellLib.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace aurora::core {

// H1-H5 — Library and technology management.
struct LibraryEntry {
  std::string name;
  std::filesystem::path path;
  int priority{0};
  bool attached{true};
  std::string techFile;     // path to tech.json
  std::string revision;     // H4 — library versioning marker
};

class LibraryManager {
 public:
  // H2/H3 — multi-library + search paths.
  void attach(const std::string& name, const std::filesystem::path& path,
              int priority = 0);
  void detach(const std::string& name);
  [[nodiscard]] LibraryEntry* find(std::string_view name);
  [[nodiscard]] const LibraryEntry* find(std::string_view name) const;
  [[nodiscard]] std::vector<LibraryEntry> attachedSorted() const;

  void addSearchPath(std::filesystem::path p);
  [[nodiscard]] const std::vector<std::filesystem::path>& searchPaths() const;

  // H1 — technology library mgmt.
  void setTechFile(const std::string& libName, const std::string& techPath);

  // H4 — versioning: stamp current revision.
  void setRevision(const std::string& libName, std::string rev);

  // H7 — Revision-control: rudimentary text-diff between two cell-lib snapshots.
  [[nodiscard]] static std::string diffSnapshots(const std::string& jsonA,
                                                  const std::string& jsonB);

  // Write/read a cds.lib-style file.
  [[nodiscard]] bool writeCdsLib(const std::filesystem::path& path) const;
  [[nodiscard]] bool readCdsLib(const std::filesystem::path& path);

 private:
  std::map<std::string, LibraryEntry, std::less<>> libs_;
  std::vector<std::filesystem::path> searchPaths_;
};

}  // namespace aurora::core
