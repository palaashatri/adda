#include "pdk/PdkManager.h"

#include <filesystem>
#include <fstream>

namespace aurora::pdk {

namespace fs = std::filesystem;

PdkInstallResult PdkManager::install(const std::string& srcDir, const std::string& installRoot) {
  PdkInstallResult r;
  std::error_code ec;
  if (!fs::exists(srcDir, ec) || !fs::is_directory(srcDir, ec)) {
    r.error = "Source not found: " + srcDir;
    return r;
  }
  fs::create_directories(installRoot, ec);
  const fs::path dst = fs::path(installRoot) / fs::path(srcDir).filename();
  fs::copy(srcDir, dst,
           fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
  if (ec) {
    r.error = "Copy failed: " + ec.message();
    return r;
  }
  r.installedPath = dst.string();
  r.success = true;
  return r;
}

PdkValidationResult PdkManager::validate(const std::string& pdkDir) {
  PdkValidationResult r;
  std::error_code ec;
  const fs::path root(pdkDir);
  if (!fs::is_directory(root, ec)) {
    r.issues.push_back("PDK dir does not exist: " + pdkDir);
    return r;
  }
  const fs::path tech = root / "tech.json";
  if (!fs::exists(tech, ec)) r.issues.push_back("Missing tech.json");
  else {
    std::ifstream in(tech);
    if (!in) r.issues.push_back("Cannot open tech.json");
    else {
      std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      if (s.find("layers") == std::string::npos) {
        r.issues.push_back("tech.json appears to lack 'layers' key");
      }
    }
  }
  // PCell dir is optional but recommended.
  if (!fs::is_directory(root / "pcells", ec)) {
    r.issues.push_back("No pcells/ directory (PCells are recommended)");
  }
  r.valid = r.issues.empty();
  return r;
}

}  // namespace aurora::pdk
