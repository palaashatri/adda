#include "db/DbConstraint.h"

#include <algorithm>
#include <utility>

namespace aurora::db {

DbConstraint::DbConstraint(DbId id, std::string type) : id_(id), type_(std::move(type)) {}

DbId DbConstraint::id() const {
  return id_;
}

const std::string& DbConstraint::type() const {
  return type_;
}

const std::vector<DbId>& DbConstraint::objectIds() const {
  return objectIds_;
}

const std::map<std::string, std::string>& DbConstraint::properties() const {
  return properties_;
}

void DbConstraint::addObject(DbId objectId) {
  if (std::find(objectIds_.begin(), objectIds_.end(), objectId) == objectIds_.end()) {
    objectIds_.push_back(objectId);
  }
}

void DbConstraint::setProperty(std::string name, std::string value) {
  properties_[std::move(name)] = std::move(value);
}

}  // namespace aurora::db
