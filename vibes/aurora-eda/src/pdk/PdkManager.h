#pragma once

#include <string>
#include <vector>

namespace aurora::pdk {

// F13 — PDK installation & F14 — PDK validation.
struct PdkInstallResult {
  bool success{false};
  std::string installedPath;
  std::string error;
};

struct PdkValidationResult {
  bool valid{false};
  std::vector<std::string> issues;
};

class PdkManager {
 public:
  // Copy a PDK directory (containing tech.json, pcells/, models/) into installRoot.
  [[nodiscard]] static PdkInstallResult install(const std::string& srcDir,
                                                 const std::string& installRoot);

  // Verify presence of required files (tech.json), parse them, optionally check PCells.
  [[nodiscard]] static PdkValidationResult validate(const std::string& pdkDir);
};

}  // namespace aurora::pdk
