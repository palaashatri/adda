#pragma once

#include "core/PluginManager.h"
#include "core/ProjectManager.h"

#include <filesystem>
#include <string_view>

namespace aurora::core {

class CoreApp {
 public:
  CoreApp() = default;

  [[nodiscard]] bool initialize(const std::filesystem::path& pluginDirectory = {});
  void shutdown();
  [[nodiscard]] bool initialized() const;

  [[nodiscard]] ProjectManager& projects();
  [[nodiscard]] const ProjectManager& projects() const;
  [[nodiscard]] PluginManager& plugins();
  [[nodiscard]] const PluginManager& plugins() const;

  [[nodiscard]] static std::string_view applicationName();
  [[nodiscard]] static std::string_view version();

 private:
  bool initialized_{false};
  ProjectManager projectManager_;
  PluginManager pluginManager_;
};

}  // namespace aurora::core
