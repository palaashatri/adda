#include "layout/LayTool.h"

#include <utility>

namespace aurora::layout {

LayTool::LayTool(std::string name) : name_(std::move(name)) {}

const std::string& LayTool::name() const {
  return name_;
}

}  // namespace aurora::layout
