#include "db/DbView.h"

#include <utility>

namespace aurora::db {

DbView::DbView(DbId id, DbId cellId, DbViewType type) : id_(id), cellId_(cellId), type_(type) {}

DbId DbView::id() const {
  return id_;
}

DbId DbView::cellId() const {
  return cellId_;
}

DbViewType DbView::type() const {
  return type_;
}

DbRect& DbView::createRect(DbId layerId, geom::GeomBox box) {
  const auto id = allocateObjectId();
  auto shape = std::make_unique<DbRect>(id, layerId, box);
  auto& ref = *shape;
  shapes_.emplace(id, std::move(shape));
  return ref;
}

DbPolygon& DbView::createPolygon(DbId layerId, geom::GeomPolygon polygon) {
  const auto id = allocateObjectId();
  auto shape = std::make_unique<DbPolygon>(id, layerId, std::move(polygon));
  auto& ref = *shape;
  shapes_.emplace(id, std::move(shape));
  return ref;
}

DbPath& DbView::createPath(DbId layerId, geom::GeomPath path) {
  const auto id = allocateObjectId();
  auto shape = std::make_unique<DbPath>(id, layerId, std::move(path));
  auto& ref = *shape;
  shapes_.emplace(id, std::move(shape));
  return ref;
}

DbText& DbView::createText(DbId layerId, geom::GeomPoint origin, std::string text) {
  const auto id = allocateObjectId();
  auto shape = std::make_unique<DbText>(id, layerId, origin, std::move(text));
  auto& ref = *shape;
  shapes_.emplace(id, std::move(shape));
  return ref;
}

DbInstance& DbView::createInstance(std::string name, DbId masterCellId, DbTransform transform) {
  const auto id = allocateObjectId();
  auto [it, inserted] =
      instances_.emplace(id, DbInstance{id, std::move(name), masterCellId, transform});
  (void)inserted;
  return it->second;
}

DbNet& DbView::createNet(std::string name) {
  const auto id = allocateObjectId();
  auto [it, inserted] = nets_.emplace(id, DbNet{id, std::move(name)});
  (void)inserted;
  return it->second;
}

DbPin& DbView::createPin(std::string name, DbPinDirection direction, DbId netId) {
  const auto id = allocateObjectId();
  auto [it, inserted] = pins_.emplace(id, DbPin{id, std::move(name), direction, netId});
  (void)inserted;

  if (netId != kInvalidId) {
    if (auto* net = findNet(netId)) {
      net->addPin(id);
    }
  }

  return it->second;
}

DbConstraint& DbView::createConstraint(std::string type) {
  const auto id = allocateObjectId();
  auto [it, inserted] = constraints_.emplace(id, DbConstraint{id, std::move(type)});
  (void)inserted;
  return it->second;
}

DbShape* DbView::findShape(DbId id) {
  auto it = shapes_.find(id);
  return it == shapes_.end() ? nullptr : it->second.get();
}

const DbShape* DbView::findShape(DbId id) const {
  auto it = shapes_.find(id);
  return it == shapes_.end() ? nullptr : it->second.get();
}

DbInstance* DbView::findInstance(DbId id) {
  auto it = instances_.find(id);
  return it == instances_.end() ? nullptr : &it->second;
}

const DbInstance* DbView::findInstance(DbId id) const {
  auto it = instances_.find(id);
  return it == instances_.end() ? nullptr : &it->second;
}

DbNet* DbView::findNet(DbId id) {
  auto it = nets_.find(id);
  return it == nets_.end() ? nullptr : &it->second;
}

const DbNet* DbView::findNet(DbId id) const {
  auto it = nets_.find(id);
  return it == nets_.end() ? nullptr : &it->second;
}

DbPin* DbView::findPin(DbId id) {
  auto it = pins_.find(id);
  return it == pins_.end() ? nullptr : &it->second;
}

const DbPin* DbView::findPin(DbId id) const {
  auto it = pins_.find(id);
  return it == pins_.end() ? nullptr : &it->second;
}

std::vector<DbId> DbView::shapeIds() const {
  std::vector<DbId> ids;
  ids.reserve(shapes_.size());
  for (const auto& [id, shape] : shapes_) {
    (void)shape;
    ids.push_back(id);
  }
  return ids;
}

std::vector<DbId> DbView::instanceIds() const {
  std::vector<DbId> ids;
  ids.reserve(instances_.size());
  for (const auto& [id, instance] : instances_) {
    (void)instance;
    ids.push_back(id);
  }
  return ids;
}

std::vector<DbId> DbView::netIds() const {
  std::vector<DbId> ids;
  ids.reserve(nets_.size());
  for (const auto& [id, net] : nets_) {
    (void)net;
    ids.push_back(id);
  }
  return ids;
}

std::vector<DbId> DbView::pinIds() const {
  std::vector<DbId> ids;
  ids.reserve(pins_.size());
  for (const auto& [id, pin] : pins_) {
    (void)pin;
    ids.push_back(id);
  }
  return ids;
}

std::vector<DbId> DbView::constraintIds() const {
  std::vector<DbId> ids;
  ids.reserve(constraints_.size());
  for (const auto& [id, constraint] : constraints_) {
    (void)constraint;
    ids.push_back(id);
  }
  return ids;
}

DbId DbView::allocateObjectId() {
  return nextObjectId_++;
}

}  // namespace aurora::db
