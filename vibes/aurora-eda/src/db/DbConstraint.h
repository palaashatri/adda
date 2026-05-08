#pragma once

#include "db/DbTypes.h"

#include <map>
#include <string>
#include <vector>

namespace aurora::db {

class DbConstraint {
 public:
  DbConstraint() = default;
  DbConstraint(DbId id, std::string type);

  [[nodiscard]] DbId id() const;
  [[nodiscard]] const std::string& type() const;
  [[nodiscard]] const std::vector<DbId>& objectIds() const;
  [[nodiscard]] const std::map<std::string, std::string>& properties() const;

  void addObject(DbId objectId);
  void setProperty(std::string name, std::string value);

 private:
  DbId id_{kInvalidId};
  std::string type_;
  std::vector<DbId> objectIds_;
  std::map<std::string, std::string> properties_;
};

}  // namespace aurora::db
