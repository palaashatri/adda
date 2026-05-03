#include "core/ProjectManager.h"

#include <fstream>

namespace aurora::core {

bool ProjectManager::createProject(const std::filesystem::path& projectPath) {
  std::error_code error;
  std::filesystem::create_directories(projectPath / "libraries", error);
  if (error) {
    return false;
  }
  std::filesystem::create_directories(projectPath / "pdk", error);
  if (error) {
    return false;
  }
  std::filesystem::create_directories(projectPath / "config", error);
  if (error) {
    return false;
  }

  currentProjectPath_ = std::filesystem::absolute(projectPath);
  return saveProject();
}

bool ProjectManager::openProject(const std::filesystem::path& projectPath) {
  if (!std::filesystem::exists(projectPath) || !std::filesystem::is_directory(projectPath)) {
    return false;
  }
  currentProjectPath_ = std::filesystem::absolute(projectPath);
  return true;
}

bool ProjectManager::saveProject() const {
  if (!currentProjectPath_) {
    return false;
  }

  const auto configDir = *currentProjectPath_ / "config";
  std::error_code error;
  std::filesystem::create_directories(configDir, error);
  if (error) {
    return false;
  }

  std::ofstream manifest(configDir / "project.json");
  if (!manifest) {
    return false;
  }

  manifest << "{\n";
  manifest << "  \"format\": \"aurora-project\",\n";
  manifest << "  \"version\": 1,\n";
  manifest << "  \"working_library\": \"" << workingLibrary_.name() << "\"\n";
  manifest << "}\n";
  return true;
}

void ProjectManager::closeProject() {
  currentProjectPath_.reset();
}

bool ProjectManager::hasOpenProject() const {
  return currentProjectPath_.has_value();
}

const std::optional<std::filesystem::path>& ProjectManager::currentProjectPath() const {
  return currentProjectPath_;
}

void ProjectManager::addLibrarySearchPath(std::filesystem::path path) {
  librarySearchPaths_.push_back(std::move(path));
}

const std::vector<std::filesystem::path>& ProjectManager::librarySearchPaths() const {
  return librarySearchPaths_;
}

db::DbCellLib& ProjectManager::workingLibrary() {
  return workingLibrary_;
}

const db::DbCellLib& ProjectManager::workingLibrary() const {
  return workingLibrary_;
}

}  // namespace aurora::core
