#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace aurora::core {

// I1 — Embedded Python shell (minimal: forwards to system python with aurora module).
// I3 — Macro recording (records user actions as Python).
// I4 — User-defined menu/toolbar callbacks.
// I5/I6 — Custom DRC rules / sim analyses registry.
// I7 — Batch / headless mode runner.

struct MacroAction {
  std::string verb;                       // e.g. "createRect", "createInstance"
  std::vector<std::string> args;          // stringified arguments
};

class MacroRecorder {
 public:
  void start();
  void stop();
  [[nodiscard]] bool isRecording() const { return recording_; }
  void record(const std::string& verb, std::vector<std::string> args);
  [[nodiscard]] const std::vector<MacroAction>& actions() const { return actions_; }
  void clear() { actions_.clear(); }

  // Emit equivalent Python script.
  [[nodiscard]] std::string toPython() const;

 private:
  bool recording_{false};
  std::vector<MacroAction> actions_;
};

struct ScriptedMenuItem {
  std::string label;
  std::string scriptPath;       // python script to run on activation
  std::string hotkey;           // optional
  std::string toolbarIcon;      // optional
};

class ScriptedUiRegistry {
 public:
  void registerItem(ScriptedMenuItem item);
  [[nodiscard]] const std::vector<ScriptedMenuItem>& items() const { return items_; }

 private:
  std::vector<ScriptedMenuItem> items_;
};

// I5/I6 — Custom rules.
struct ScriptedRule {
  std::string name;
  std::string scriptPath;
  std::string kind;             // "drc" | "sim"
};

class CustomRuleRegistry {
 public:
  void registerRule(ScriptedRule rule);
  [[nodiscard]] std::vector<ScriptedRule> rulesByKind(const std::string& kind) const;

 private:
  std::vector<ScriptedRule> rules_;
};

// I7 — Batch / headless mode.
struct BatchOptions {
  std::filesystem::path scriptPath;
  std::map<std::string, std::string> args;
  bool exportResults{false};
  std::filesystem::path resultsDir;
};

class BatchRunner {
 public:
  // Returns exit code from invoked Python script. Spawns `python <scriptPath>`.
  [[nodiscard]] static int run(const BatchOptions& opts);
};

}  // namespace aurora::core
