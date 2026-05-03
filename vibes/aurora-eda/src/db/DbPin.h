#pragma once

#include "db/DbTypes.h"

#include <string>
#include <vector>

namespace aurora::db {

class DbPin {
 public:
  DbPin() = default;
  DbPin(DbId id, std::string name, DbPinDirection direction, DbId netId = kInvalidId);

  [[nodiscard]] DbId id() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] DbPinDirection direction() const;
  [[nodiscard]] DbId netId() const;
  [[nodiscard]] const std::vector<DbId>& shapeIds() const;

  void setName(std::string name);
  void setDirection(DbPinDirection direction);
  void setNetId(DbId netId);
  void addShape(DbId shapeId);

 private:
  DbId id_{kInvalidId};
  std::string name_;
  DbPinDirection direction_{DbPinDirection::Unknown};
  DbId netId_{kInvalidId};
  std::vector<DbId> shapeIds_;
};

}  // namespace aurora::db
