#pragma once

#include "db/DbCellLib.h"

#include <filesystem>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace aurora::core {

// H5 — Design hierarchy browser.
struct HierarchyNode {
  std::string cellName;
  std::vector<HierarchyNode> children;
  std::set<std::string> usedBy;
};

class HierarchyBrowser {
 public:
  [[nodiscard]] static HierarchyNode build(const db::DbCellLib& lib,
                                            const std::string& topCell);
  [[nodiscard]] static std::vector<std::string> findUsage(const db::DbCellLib& lib,
                                                           const std::string& cellName);
};

// H8 — Project archiving (single-file bundle).
class ProjectArchiver {
 public:
  [[nodiscard]] static bool archive(const std::filesystem::path& projectDir,
                                     const std::filesystem::path& outFile);
  [[nodiscard]] static bool extract(const std::filesystem::path& archiveFile,
                                     const std::filesystem::path& destDir);
};

// H9 — Design health dashboard.
struct DesignHealthReport {
  std::size_t cellCount{0};
  std::size_t shapeCount{0};
  std::size_t instanceCount{0};
  std::size_t netCount{0};
  std::size_t warningCount{0};
  std::size_t errorCount{0};
  bool lastDrcPassed{false};
  bool lastLvsPassed{false};
  std::map<std::string, std::size_t> cellsPerView;
};

class DesignHealthDashboard {
 public:
  [[nodiscard]] static DesignHealthReport compile(const db::DbCellLib& lib);
};

}  // namespace aurora::core
