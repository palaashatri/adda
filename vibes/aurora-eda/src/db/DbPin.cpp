#include "db/DbPin.h"

#include <algorithm>
#include <utility>

namespace aurora::db {

DbPin::DbPin(DbId id, std::string name, DbPinDirection direction, DbId netId)
    : id_(id), name_(std::move(name)), direction_(direction), netId_(netId) {}

DbId DbPin::id() const {
  return id_;
}

const std::string& DbPin::name() const {
  return name_;
}

DbPinDirection DbPin::direction() const {
  return direction_;
}

DbId DbPin::netId() const {
  return netId_;
}

const std::vector<DbId>& DbPin::shapeIds() const {
  return shapeIds_;
}

void DbPin::setName(std::string name) {
  name_ = std::move(name);
}

void DbPin::setDirection(DbPinDirection direction) {
  direction_ = direction;
}

void DbPin::setNetId(DbId netId) {
  netId_ = netId;
}

void DbPin::addShape(DbId shapeId) {
  if (std::find(shapeIds_.begin(), shapeIds_.end(), shapeId) == shapeIds_.end()) {
    shapeIds_.push_back(shapeId);
  }
}

}  // namespace aurora::db
