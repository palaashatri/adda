#pragma once

#include "core/PluginApi.h"

#include <filesystem>
#include <string>
#include <vector>

namespace aurora::core {

class PluginManager {
 public:
  PluginManager() = default;
  ~PluginManager();

  PluginManager(const PluginManager&) = delete;
  PluginManager& operator=(const PluginManager&) = delete;
  PluginManager(PluginManager&&) = delete;
  PluginManager& operator=(PluginManager&&) = delete;

  [[nodiscard]] bool loadPlugin(const std::filesystem::path& path);
  [[nodiscard]] std::size_t loadPluginsFromDirectory(const std::filesystem::path& directory);

  [[nodiscard]] PluginRegistry& registry();
  [[nodiscard]] const PluginRegistry& registry() const;
  [[nodiscard]] const std::vector<std::filesystem::path>& loadedPluginPaths() const;
  [[nodiscard]] const std::string& lastError() const;

 private:
  struct LoadedPlugin {
    std::filesystem::path path;
    void* nativeHandle{nullptr};
  };

  void unloadAll();
  void setLastError(std::string error);

  PluginRegistry registry_;
  std::vector<LoadedPlugin> loadedPlugins_;
  std::vector<std::filesystem::path> loadedPluginPaths_;
  std::string lastError_;
};

}  // namespace aurora::core
