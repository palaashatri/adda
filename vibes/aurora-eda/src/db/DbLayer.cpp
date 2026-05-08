#include "db/DbLayer.h"

#include <utility>

namespace aurora::db {

DbLayer::DbLayer(DbId id, std::string name, std::string purpose)
    : id_(id), name_(std::move(name)), purpose_(std::move(purpose)) {}

DbId DbLayer::id() const {
  return id_;
}

const std::string& DbLayer::name() const {
  return name_;
}

const std::string& DbLayer::purpose() const {
  return purpose_;
}

const std::string& DbLayer::color() const {
  return color_;
}

int DbLayer::gdsLayer() const {
  return gdsLayer_;
}

int DbLayer::gdsDatatype() const {
  return gdsDatatype_;
}

void DbLayer::setColor(std::string color) {
  color_ = std::move(color);
}

void DbLayer::setGdsMapping(int layer, int datatype) {
  gdsLayer_ = layer;
  gdsDatatype_ = datatype;
}

}  // namespace aurora::db
