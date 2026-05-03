#pragma once

#include <string>

namespace aurora::layout {

class LayTool {
 public:
  explicit LayTool(std::string name = {});
  virtual ~LayTool() = default;

  [[nodiscard]] const std::string& name() const;

 private:
  std::string name_;
};

}  // namespace aurora::layout
