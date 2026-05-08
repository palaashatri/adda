#include "db/DbNet.h"

#include <algorithm>
#include <utility>

namespace aurora::db {

DbNet::DbNet(DbId id, std::string name) : id_(id), name_(std::move(name)) {}

DbId DbNet::id() const {
  return id_;
}

const std::string& DbNet::name() const {
  return name_;
}

const std::vector<DbId>& DbNet::pinIds() const {
  return pinIds_;
}

const std::map<std::string, std::string>& DbNet::properties() const {
  return properties_;
}

void DbNet::setName(std::string name) {
  name_ = std::move(name);
}

void DbNet::addPin(DbId pinId) {
  if (std::find(pinIds_.begin(), pinIds_.end(), pinId) == pinIds_.end()) {
    pinIds_.push_back(pinId);
  }
}

void DbNet::removePin(DbId pinId) {
  pinIds_.erase(std::remove(pinIds_.begin(), pinIds_.end(), pinId), pinIds_.end());
}

void DbNet::setProperty(std::string name, std::string value) {
  properties_[std::move(name)] = std::move(value);
}

}  // namespace aurora::db
