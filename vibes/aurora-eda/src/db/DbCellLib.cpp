#include "db/DbCellLib.h"

#include <stdexcept>
#include <utility>

namespace aurora::db {

DbCellLib::DbCellLib(std::string name) : name_(std::move(name)) {}

const std::string& DbCellLib::name() const {
  return name_;
}

void DbCellLib::setName(std::string name) {
  name_ = std::move(name);
}

DbCell& DbCellLib::createCell(std::string name) {
  if (auto* existing = findCell(name)) {
    return *existing;
  }

  const auto id = allocateCellId();
  auto cell = std::make_unique<DbCell>(id, name);
  auto& ref = *cell;
  cellNameIndex_[cell->name()] = id;
  cells_.emplace(id, std::move(cell));
  return ref;
}

DbCell* DbCellLib::findCell(std::string_view name) {
  const auto it = cellNameIndex_.find(name);
  return it == cellNameIndex_.end() ? nullptr : findCellById(it->second);
}

const DbCell* DbCellLib::findCell(std::string_view name) const {
  const auto it = cellNameIndex_.find(name);
  return it == cellNameIndex_.end() ? nullptr : findCellById(it->second);
}

DbCell* DbCellLib::findCellById(DbId id) {
  const auto it = cells_.find(id);
  return it == cells_.end() ? nullptr : it->second.get();
}

const DbCell* DbCellLib::findCellById(DbId id) const {
  const auto it = cells_.find(id);
  return it == cells_.end() ? nullptr : it->second.get();
}

std::vector<DbId> DbCellLib::cellIds() const {
  std::vector<DbId> ids;
  ids.reserve(cells_.size());
  for (const auto& [id, cell] : cells_) {
    (void)cell;
    ids.push_back(id);
  }
  return ids;
}

DbLayer& DbCellLib::createLayer(std::string name, std::string purpose) {
  const auto id = allocateLayerId();
  auto [it, inserted] = layers_.emplace(id, DbLayer{id, std::move(name), std::move(purpose)});
  (void)inserted;
  return it->second;
}

DbLayer* DbCellLib::findLayer(DbId id) {
  const auto it = layers_.find(id);
  return it == layers_.end() ? nullptr : &it->second;
}

const DbLayer* DbCellLib::findLayer(DbId id) const {
  const auto it = layers_.find(id);
  return it == layers_.end() ? nullptr : &it->second;
}

std::vector<DbId> DbCellLib::layerIds() const {
  std::vector<DbId> ids;
  ids.reserve(layers_.size());
  for (const auto& [id, layer] : layers_) {
    (void)layer;
    ids.push_back(id);
  }
  return ids;
}

DbId DbCellLib::allocateCellId() {
  return nextCellId_++;
}

DbId DbCellLib::allocateLayerId() {
  return nextLayerId_++;
}

}  // namespace aurora::db
