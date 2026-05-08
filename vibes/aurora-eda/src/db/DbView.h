#pragma once

#include "db/DbConstraint.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbShape.h"
#include "db/DbTypes.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace aurora::db {

class DbView {
 public:
  DbView() = default;
  DbView(DbId id, DbId cellId, DbViewType type);

  [[nodiscard]] DbId id() const;
  [[nodiscard]] DbId cellId() const;
  [[nodiscard]] DbViewType type() const;

  [[nodiscard]] DbRect& createRect(DbId layerId, geom::GeomBox box);
  [[nodiscard]] DbPolygon& createPolygon(DbId layerId, geom::GeomPolygon polygon);
  [[nodiscard]] DbPath& createPath(DbId layerId, geom::GeomPath path);
  [[nodiscard]] DbText& createText(DbId layerId, geom::GeomPoint origin, std::string text);
  [[nodiscard]] DbInstance& createInstance(std::string name, DbId masterCellId,
                                           DbTransform transform = {});
  [[nodiscard]] DbNet& createNet(std::string name);
  [[nodiscard]] DbPin& createPin(std::string name, DbPinDirection direction,
                                 DbId netId = kInvalidId);
  [[nodiscard]] DbConstraint& createConstraint(std::string type);

  [[nodiscard]] DbShape* findShape(DbId id);
  [[nodiscard]] const DbShape* findShape(DbId id) const;
  [[nodiscard]] DbInstance* findInstance(DbId id);
  [[nodiscard]] const DbInstance* findInstance(DbId id) const;
  [[nodiscard]] DbNet* findNet(DbId id);
  [[nodiscard]] const DbNet* findNet(DbId id) const;
  [[nodiscard]] DbPin* findPin(DbId id);
  [[nodiscard]] const DbPin* findPin(DbId id) const;

  [[nodiscard]] std::vector<DbId> shapeIds() const;
  [[nodiscard]] std::vector<DbId> instanceIds() const;
  [[nodiscard]] std::vector<DbId> netIds() const;
  [[nodiscard]] std::vector<DbId> pinIds() const;
  [[nodiscard]] std::vector<DbId> constraintIds() const;

 private:
  [[nodiscard]] DbId allocateObjectId();

  DbId id_{kInvalidId};
  DbId cellId_{kInvalidId};
  DbViewType type_{DbViewType::Schematic};
  DbId nextObjectId_{1};
  std::map<DbId, std::unique_ptr<DbShape>> shapes_;
  std::map<DbId, DbInstance> instances_;
  std::map<DbId, DbNet> nets_;
  std::map<DbId, DbPin> pins_;
  std::map<DbId, DbConstraint> constraints_;
};

}  // namespace aurora::db
