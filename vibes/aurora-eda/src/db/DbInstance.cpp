#include "db/DbInstance.h"

#include <utility>

namespace aurora::db {

DbInstance::DbInstance(DbId id, std::string name, DbId masterCellId, DbTransform transform)
    : id_(id),
      name_(std::move(name)),
      masterCellId_(masterCellId),
      transform_(transform) {}

DbId DbInstance::id() const {
  return id_;
}

const std::string& DbInstance::name() const {
  return name_;
}

DbId DbInstance::masterCellId() const {
  return masterCellId_;
}

const DbTransform& DbInstance::transform() const {
  return transform_;
}

const std::map<std::string, std::string>& DbInstance::parameters() const {
  return parameters_;
}

void DbInstance::setName(std::string name) {
  name_ = std::move(name);
}

void DbInstance::setMasterCellId(DbId masterCellId) {
  masterCellId_ = masterCellId;
}

void DbInstance::setTransform(DbTransform transform) {
  transform_ = transform;
}

void DbInstance::setParameter(std::string name, std::string value) {
  parameters_[std::move(name)] = std::move(value);
}

}  // namespace aurora::db
