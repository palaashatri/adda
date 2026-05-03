#pragma once

#include "db/DbCell.h"
#include "db/DbLayer.h"
#include "db/DbTypes.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace aurora::db {

class DbCellLib {
 public:
  DbCellLib() = default;
  explicit DbCellLib(std::string name);

  [[nodiscard]] const std::string& name() const;
  void setName(std::string name);

  [[nodiscard]] DbCell& createCell(std::string name);
  [[nodiscard]] DbCell* findCell(std::string_view name);
  [[nodiscard]] const DbCell* findCell(std::string_view name) const;
  [[nodiscard]] DbCell* findCellById(DbId id);
  [[nodiscard]] const DbCell* findCellById(DbId id) const;
  [[nodiscard]] std::vector<DbId> cellIds() const;

  [[nodiscard]] DbLayer& createLayer(std::string name, std::string purpose);
  [[nodiscard]] DbLayer* findLayer(DbId id);
  [[nodiscard]] const DbLayer* findLayer(DbId id) const;
  [[nodiscard]] std::vector<DbId> layerIds() const;

 private:
  [[nodiscard]] DbId allocateCellId();
  [[nodiscard]] DbId allocateLayerId();

  std::string name_;
  DbId nextCellId_{1};
  DbId nextLayerId_{1};
  std::map<DbId, std::unique_ptr<DbCell>> cells_;
  std::map<std::string, DbId, std::less<>> cellNameIndex_;
  std::map<DbId, DbLayer> layers_;
};

}  // namespace aurora::db
