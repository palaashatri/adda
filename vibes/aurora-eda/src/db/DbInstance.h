#pragma once

#include "db/DbTypes.h"
#include "geom/GeomPoint.h"

#include <map>
#include <string>

namespace aurora::db {

struct DbTransform {
  geom::DbUnit dx{0};
  geom::DbUnit dy{0};
  int rotationDegrees{0};
  bool mirrorX{false};
};

class DbInstance {
 public:
  DbInstance() = default;
  DbInstance(DbId id, std::string name, DbId masterCellId, DbTransform transform = {});

  [[nodiscard]] DbId id() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] DbId masterCellId() const;
  [[nodiscard]] const DbTransform& transform() const;
  [[nodiscard]] const std::map<std::string, std::string>& parameters() const;

  void setName(std::string name);
  void setMasterCellId(DbId masterCellId);
  void setTransform(DbTransform transform);
  void setParameter(std::string name, std::string value);

 private:
  DbId id_{kInvalidId};
  std::string name_;
  DbId masterCellId_{kInvalidId};
  DbTransform transform_;
  std::map<std::string, std::string> parameters_;
};

}  // namespace aurora::db
