#pragma once

#include "db/DbCellLib.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace aurora::core {

class ProjectManager {
 public:
  [[nodiscard]] bool createProject(const std::filesystem::path& projectPath);
  [[nodiscard]] bool openProject(const std::filesystem::path& projectPath);
  [[nodiscard]] bool saveProject() const;
  void closeProject();

  [[nodiscard]] bool hasOpenProject() const;
  [[nodiscard]] const std::optional<std::filesystem::path>& currentProjectPath() const;

  void addLibrarySearchPath(std::filesystem::path path);
  [[nodiscard]] const std::vector<std::filesystem::path>& librarySearchPaths() const;

  [[nodiscard]] db::DbCellLib& workingLibrary();
  [[nodiscard]] const db::DbCellLib& workingLibrary() const;

 private:
  std::optional<std::filesystem::path> currentProjectPath_;
  std::vector<std::filesystem::path> librarySearchPaths_;
  db::DbCellLib workingLibrary_{"worklib"};
};

}  // namespace aurora::core
