#include "db/DbCell.h"

#include <stdexcept>
#include <utility>

namespace aurora::db {

DbCell::DbCell(DbId id, std::string name) : id_(id), name_(std::move(name)) {}

DbId DbCell::id() const {
  return id_;
}

const std::string& DbCell::name() const {
  return name_;
}

const std::map<std::string, std::string>& DbCell::parameters() const {
  return parameters_;
}

void DbCell::setName(std::string name) {
  name_ = std::move(name);
}

void DbCell::setParameter(std::string name, std::string value) {
  parameters_[std::move(name)] = std::move(value);
}

DbView& DbCell::createView(DbViewType type) {
  if (auto* existing = findView(type)) {
    return *existing;
  }

  const auto id = allocateViewId();
  auto view = std::make_unique<DbView>(id, id_, type);
  auto& ref = *view;
  views_.emplace(id, std::move(view));
  viewIndex_[type] = id;
  return ref;
}

DbView* DbCell::findView(DbViewType type) {
  const auto it = viewIndex_.find(type);
  return it == viewIndex_.end() ? nullptr : findViewById(it->second);
}

const DbView* DbCell::findView(DbViewType type) const {
  const auto it = viewIndex_.find(type);
  return it == viewIndex_.end() ? nullptr : findViewById(it->second);
}

DbView* DbCell::findViewById(DbId id) {
  const auto it = views_.find(id);
  return it == views_.end() ? nullptr : it->second.get();
}

const DbView* DbCell::findViewById(DbId id) const {
  const auto it = views_.find(id);
  return it == views_.end() ? nullptr : it->second.get();
}

std::vector<DbId> DbCell::viewIds() const {
  std::vector<DbId> ids;
  ids.reserve(views_.size());
  for (const auto& [id, view] : views_) {
    (void)view;
    ids.push_back(id);
  }
  return ids;
}

DbId DbCell::allocateViewId() {
  return nextViewId_++;
}

}  // namespace aurora::db
