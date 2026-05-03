#include "db/DbShape.h"

#include <utility>

namespace aurora::db {

DbShape::DbShape(DbId id, DbId layerId) : id_(id), layerId_(layerId) {}

DbId DbShape::id() const {
  return id_;
}

DbId DbShape::layerId() const {
  return layerId_;
}

void DbShape::setLayerId(DbId layerId) {
  layerId_ = layerId;
}

DbRect::DbRect(DbId id, DbId layerId, geom::GeomBox box) : DbShape(id, layerId), box_(box) {}

DbShapeKind DbRect::kind() const {
  return DbShapeKind::Rect;
}

std::unique_ptr<DbShape> DbRect::clone() const {
  return std::make_unique<DbRect>(*this);
}

const geom::GeomBox& DbRect::box() const {
  return box_;
}

void DbRect::setBox(geom::GeomBox box) {
  box_ = box;
}

DbPolygon::DbPolygon(DbId id, DbId layerId, geom::GeomPolygon polygon)
    : DbShape(id, layerId), polygon_(std::move(polygon)) {}

DbShapeKind DbPolygon::kind() const {
  return DbShapeKind::Polygon;
}

std::unique_ptr<DbShape> DbPolygon::clone() const {
  return std::make_unique<DbPolygon>(*this);
}

const geom::GeomPolygon& DbPolygon::polygon() const {
  return polygon_;
}

void DbPolygon::setPolygon(geom::GeomPolygon polygon) {
  polygon_ = std::move(polygon);
}

DbPath::DbPath(DbId id, DbId layerId, geom::GeomPath path)
    : DbShape(id, layerId), path_(std::move(path)) {}

DbShapeKind DbPath::kind() const {
  return DbShapeKind::Path;
}

std::unique_ptr<DbShape> DbPath::clone() const {
  return std::make_unique<DbPath>(*this);
}

const geom::GeomPath& DbPath::path() const {
  return path_;
}

void DbPath::setPath(geom::GeomPath path) {
  path_ = std::move(path);
}

DbText::DbText(DbId id, DbId layerId, geom::GeomPoint origin, std::string text)
    : DbShape(id, layerId), origin_(origin), text_(std::move(text)) {}

DbShapeKind DbText::kind() const {
  return DbShapeKind::Text;
}

std::unique_ptr<DbShape> DbText::clone() const {
  return std::make_unique<DbText>(*this);
}

geom::GeomPoint DbText::origin() const {
  return origin_;
}

const std::string& DbText::text() const {
  return text_;
}

void DbText::setOrigin(geom::GeomPoint origin) {
  origin_ = origin;
}

void DbText::setText(std::string text) {
  text_ = std::move(text);
}

}  // namespace aurora::db
