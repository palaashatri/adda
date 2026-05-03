#include "core/PluginManager.h"

#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace aurora::core {

void PluginRegistry::registerImporter(std::string name, std::vector<std::string> extensions) {
  services_.push_back({"importer", std::move(name), std::move(extensions)});
}

void PluginRegistry::registerExporter(std::string name, std::vector<std::string> extensions) {
  services_.push_back({"exporter", std::move(name), std::move(extensions)});
}

void PluginRegistry::registerTool(std::string name) {
  services_.push_back({"tool", std::move(name), {}});
}

const std::vector<PluginService>& PluginRegistry::services() const {
  return services_;
}

PluginManager::~PluginManager() {
  unloadAll();
}

bool PluginManager::loadPlugin(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    setLastError("Plugin does not exist: " + path.string());
    return false;
  }

#if defined(_WIN32)
  auto* handle = reinterpret_cast<void*>(LoadLibraryW(path.wstring().c_str()));
  if (handle == nullptr) {
    setLastError("Failed to load plugin: " + path.string());
    return false;
  }
  auto* symbol = reinterpret_cast<PluginRegisterFn>(
      GetProcAddress(reinterpret_cast<HMODULE>(handle), "aurora_register_plugin"));
#else
  void* handle = dlopen(path.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    setLastError(dlerror() == nullptr ? "Failed to load plugin" : dlerror());
    return false;
  }
  auto* symbol = reinterpret_cast<PluginRegisterFn>(dlsym(handle, "aurora_register_plugin"));
#endif

  if (symbol == nullptr) {
    setLastError("Plugin is missing aurora_register_plugin: " + path.string());
#if defined(_WIN32)
    FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
    return false;
  }

  symbol(registry_);
  loadedPluginPaths_.push_back(std::filesystem::absolute(path));
  loadedPlugins_.push_back({std::filesystem::absolute(path), handle});
  lastError_.clear();
  return true;
}

std::size_t PluginManager::loadPluginsFromDirectory(const std::filesystem::path& directory) {
  if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
    return 0;
  }

  std::size_t loaded = 0;
  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (loadPlugin(entry.path())) {
      ++loaded;
    }
  }
  return loaded;
}

PluginRegistry& PluginManager::registry() {
  return registry_;
}

const PluginRegistry& PluginManager::registry() const {
  return registry_;
}

const std::vector<std::filesystem::path>& PluginManager::loadedPluginPaths() const {
  return loadedPluginPaths_;
}

const std::string& PluginManager::lastError() const {
  return lastError_;
}

void PluginManager::unloadAll() {
  for (auto& plugin : loadedPlugins_) {
    if (plugin.nativeHandle == nullptr) {
      continue;
    }
#if defined(_WIN32)
    FreeLibrary(reinterpret_cast<HMODULE>(plugin.nativeHandle));
#else
    dlclose(plugin.nativeHandle);
#endif
    plugin.nativeHandle = nullptr;
  }
  loadedPlugins_.clear();
  loadedPluginPaths_.clear();
}

void PluginManager::setLastError(std::string error) {
  lastError_ = std::move(error);
}

}  // namespace aurora::core
