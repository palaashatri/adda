#pragma once

#include "db/DbTypes.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace aurora::db {
class DbCellLib;
class DbView;
}

namespace aurora::core {

// J1 — Customizable workspace layout persistence.
struct DockPosition {
  std::string name;
  std::string area;    // "left", "right", "top", "bottom", "center"
  int width{200};
  int height{200};
  bool visible{true};
};

class WorkspaceLayout {
 public:
  void setDock(DockPosition d);
  [[nodiscard]] const std::vector<DockPosition>& docks() const { return docks_; }
  [[nodiscard]] bool save(const std::filesystem::path& path) const;
  [[nodiscard]] bool load(const std::filesystem::path& path);

 private:
  std::vector<DockPosition> docks_;
};

// J2 — Theme management.
struct Theme {
  std::string name;
  std::string background{"#1e1e1e"};
  std::string foreground{"#dcdcdc"};
  std::string accent{"#3a8ee6"};
  std::string grid{"#404040"};
  std::map<std::string, std::string> layerColors;
};

class ThemeManager {
 public:
  [[nodiscard]] const Theme& currentTheme() const { return current_; }
  void apply(Theme t) { current_ = std::move(t); }
  [[nodiscard]] static Theme dark();
  [[nodiscard]] static Theme light();

 private:
  Theme current_;
};

// J4 — Search and replace.
struct SearchHit {
  std::string cellName;
  std::string kind;        // "net" | "instance" | "shape"
  std::string objectName;
  db::DbId viewId{db::kInvalidId};
};

class DesignSearch {
 public:
  [[nodiscard]] static std::vector<SearchHit> findNet(const db::DbCellLib& lib,
                                                       const std::string& pattern);
  [[nodiscard]] static std::vector<SearchHit> findInstance(const db::DbCellLib& lib,
                                                            const std::string& pattern);
  [[nodiscard]] static std::size_t replaceNetName(db::DbCellLib& lib,
                                                   const std::string& from,
                                                   const std::string& to);
};

// J5 — Tech rule table editor backend.
struct TechRuleEntry {
  std::string layer;
  std::string ruleKind;    // "min_width" | "min_spacing" | ...
  double value{0.0};
  std::string unit{"nm"};
};

class TechRuleEditor {
 public:
  void setRule(TechRuleEntry e);
  [[nodiscard]] const std::vector<TechRuleEntry>& rules() const { return rules_; }
  [[nodiscard]] bool exportJson(const std::filesystem::path& path) const;

 private:
  std::vector<TechRuleEntry> rules_;
};

// J7 — Hotkey/macro configuration.
class HotkeyConfig {
 public:
  void bind(const std::string& action, const std::string& key);
  [[nodiscard]] std::string keyFor(const std::string& action) const;
  [[nodiscard]] const std::map<std::string, std::string, std::less<>>& bindings() const {
    return bindings_;
  }
  [[nodiscard]] bool save(const std::filesystem::path& path) const;
  [[nodiscard]] bool load(const std::filesystem::path& path);

 private:
  std::map<std::string, std::string, std::less<>> bindings_;
};

// J8 — Startup wizard data.
struct StartupSelection {
  std::filesystem::path projectDir;
  std::filesystem::path pdkDir;
  std::string projectName;
  std::string techName;
};

// J9 — Progress reporting.
class ProgressReporter {
 public:
  void begin(const std::string& task, std::size_t total) {
    task_ = task; total_ = total; done_ = 0;
  }
  void advance(std::size_t n = 1) { done_ += n; }
  void end() { done_ = total_; }
  [[nodiscard]] double fraction() const {
    return total_ == 0 ? 1.0 : static_cast<double>(done_) / static_cast<double>(total_);
  }
  [[nodiscard]] const std::string& task() const { return task_; }

 private:
  std::string task_;
  std::size_t total_{0};
  std::size_t done_{0};
};

// J10 — Notification center.
enum class NotificationLevel { Info, Warning, Error, Success };

struct Notification {
  std::string title;
  std::string body;
  NotificationLevel level{NotificationLevel::Info};
  long long timestamp{0};
};

class NotificationCenter {
 public:
  void post(Notification n);
  [[nodiscard]] const std::vector<Notification>& items() const { return items_; }
  void clear() { items_.clear(); }

 private:
  std::vector<Notification> items_;
};

}  // namespace aurora::core
