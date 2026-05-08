#include "core/CoreApp.h"

namespace aurora::core {

bool CoreApp::initialize(const std::filesystem::path& pluginDirectory) {
  if (initialized_) {
    return true;
  }

  if (!pluginDirectory.empty()) {
    (void)pluginManager_.loadPluginsFromDirectory(pluginDirectory);
  }

  initialized_ = true;
  return initialized_;
}

void CoreApp::shutdown() {
  projectManager_.closeProject();
  initialized_ = false;
}

bool CoreApp::initialized() const {
  return initialized_;
}

ProjectManager& CoreApp::projects() {
  return projectManager_;
}

const ProjectManager& CoreApp::projects() const {
  return projectManager_;
}

PluginManager& CoreApp::plugins() {
  return pluginManager_;
}

const PluginManager& CoreApp::plugins() const {
  return pluginManager_;
}

std::string_view CoreApp::applicationName() {
  return "aurora-eda";
}

std::string_view CoreApp::version() {
  return "0.1.0-dev";
}

}  // namespace aurora::core
