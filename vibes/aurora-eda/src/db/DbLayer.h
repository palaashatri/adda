#pragma once

#include "db/DbTypes.h"

#include <string>

namespace aurora::db {

class DbLayer {
 public:
  DbLayer() = default;
  DbLayer(DbId id, std::string name, std::string purpose);

  [[nodiscard]] DbId id() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] const std::string& purpose() const;
  [[nodiscard]] const std::string& color() const;
  [[nodiscard]] int gdsLayer() const;
  [[nodiscard]] int gdsDatatype() const;

  void setColor(std::string color);
  void setGdsMapping(int layer, int datatype);

 private:
  DbId id_{kInvalidId};
  std::string name_;
  std::string purpose_;
  std::string color_{"#808080"};
  int gdsLayer_{-1};
  int gdsDatatype_{-1};
};

}  // namespace aurora::db
