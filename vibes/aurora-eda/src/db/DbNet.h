#pragma once

#include "db/DbTypes.h"

#include <map>
#include <string>
#include <vector>

namespace aurora::db {

class DbNet {
 public:
  DbNet() = default;
  DbNet(DbId id, std::string name);

  [[nodiscard]] DbId id() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] const std::vector<DbId>& pinIds() const;
  [[nodiscard]] const std::map<std::string, std::string>& properties() const;

  void setName(std::string name);
  void addPin(DbId pinId);
  void removePin(DbId pinId);
  void setProperty(std::string name, std::string value);

 private:
  DbId id_{kInvalidId};
  std::string name_;
  std::vector<DbId> pinIds_;
  std::map<std::string, std::string> properties_;
};

}  // namespace aurora::db
