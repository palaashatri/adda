#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace aurora::core {

struct PluginService {
  std::string kind;
  std::string name;
  std::vector<std::string> extensions;
};

class PluginRegistry {
 public:
  void registerImporter(std::string name, std::vector<std::string> extensions);
  void registerExporter(std::string name, std::vector<std::string> extensions);
  void registerTool(std::string name);

  [[nodiscard]] const std::vector<PluginService>& services() const;

 private:
  std::vector<PluginService> services_;
};

using PluginRegisterFn = void (*)(PluginRegistry& registry);

}  // namespace aurora::core
