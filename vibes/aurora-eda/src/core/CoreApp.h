#pragma once

#include "core/PluginManager.h"
#include "core/ProjectManager.h"
#include "pdk/PcellRegistry.h"
#include "tech/TechDatabase.h"

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
  [[nodiscard]] tech::TechDatabase& tech();
  [[nodiscard]] const tech::TechDatabase& tech() const;
  [[nodiscard]] pdk::PcellRegistry& pcells();
  [[nodiscard]] const pdk::PcellRegistry& pcells() const;

  [[nodiscard]] static std::string_view applicationName();
  [[nodiscard]] static std::string_view version();

 private:
  bool initialized_{false};
  ProjectManager projectManager_;
  PluginManager pluginManager_;
  tech::TechDatabase techDatabase_;
  pdk::PcellRegistry pcellRegistry_;
};

}  // namespace aurora::core
