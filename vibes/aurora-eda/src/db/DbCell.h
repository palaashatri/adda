#pragma once

#include "db/DbTypes.h"
#include "db/DbView.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace aurora::db {

class DbCell {
 public:
  DbCell() = default;
  DbCell(DbId id, std::string name);

  [[nodiscard]] DbId id() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] const std::map<std::string, std::string>& parameters() const;

  void setName(std::string name);
  void setParameter(std::string name, std::string value);

  [[nodiscard]] DbView& createView(DbViewType type);
  [[nodiscard]] DbView* findView(DbViewType type);
  [[nodiscard]] const DbView* findView(DbViewType type) const;
  [[nodiscard]] DbView* findViewById(DbId id);
  [[nodiscard]] const DbView* findViewById(DbId id) const;
  [[nodiscard]] std::vector<DbId> viewIds() const;

 private:
  [[nodiscard]] DbId allocateViewId();

  DbId id_{kInvalidId};
  std::string name_;
  DbId nextViewId_{1};
  std::map<std::string, std::string> parameters_;
  std::map<DbId, std::unique_ptr<DbView>> views_;
  std::map<DbViewType, DbId> viewIndex_;
};

}  // namespace aurora::db
