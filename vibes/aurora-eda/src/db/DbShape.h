#pragma once

#include "db/DbTypes.h"
#include "geom/GeomBox.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"
#include "geom/GeomPolygon.h"

#include <memory>
#include <string>

namespace aurora::db {

class DbShape {
 public:
  DbShape(DbId id, DbId layerId);
  virtual ~DbShape() = default;

  DbShape(const DbShape&) = default;
  DbShape& operator=(const DbShape&) = default;
  DbShape(DbShape&&) noexcept = default;
  DbShape& operator=(DbShape&&) noexcept = default;

  [[nodiscard]] DbId id() const;
  [[nodiscard]] DbId layerId() const;
  void setLayerId(DbId layerId);

  [[nodiscard]] virtual DbShapeKind kind() const = 0;
  [[nodiscard]] virtual std::unique_ptr<DbShape> clone() const = 0;

 private:
  DbId id_{kInvalidId};
  DbId layerId_{kInvalidId};
};

class DbRect final : public DbShape {
 public:
  DbRect(DbId id, DbId layerId, geom::GeomBox box);

  [[nodiscard]] DbShapeKind kind() const override;
  [[nodiscard]] std::unique_ptr<DbShape> clone() const override;
  [[nodiscard]] const geom::GeomBox& box() const;
  void setBox(geom::GeomBox box);

 private:
  geom::GeomBox box_;
};

class DbPolygon final : public DbShape {
 public:
  DbPolygon(DbId id, DbId layerId, geom::GeomPolygon polygon);

  [[nodiscard]] DbShapeKind kind() const override;
  [[nodiscard]] std::unique_ptr<DbShape> clone() const override;
  [[nodiscard]] const geom::GeomPolygon& polygon() const;
  void setPolygon(geom::GeomPolygon polygon);

 private:
  geom::GeomPolygon polygon_;
};

class DbPath final : public DbShape {
 public:
  DbPath(DbId id, DbId layerId, geom::GeomPath path);

  [[nodiscard]] DbShapeKind kind() const override;
  [[nodiscard]] std::unique_ptr<DbShape> clone() const override;
  [[nodiscard]] const geom::GeomPath& path() const;
  void setPath(geom::GeomPath path);

 private:
  geom::GeomPath path_;
};

class DbText final : public DbShape {
 public:
  DbText(DbId id, DbId layerId, geom::GeomPoint origin, std::string text);

  [[nodiscard]] DbShapeKind kind() const override;
  [[nodiscard]] std::unique_ptr<DbShape> clone() const override;
  [[nodiscard]] geom::GeomPoint origin() const;
  [[nodiscard]] const std::string& text() const;

  void setOrigin(geom::GeomPoint origin);
  void setText(std::string text);

 private:
  geom::GeomPoint origin_;
  std::string text_;
};

}  // namespace aurora::db
