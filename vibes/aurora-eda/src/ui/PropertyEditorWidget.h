#pragma once

#include "db/DbTypes.h"

#include <QWidget>

class QFormLayout;
class QLabel;
class QScrollArea;

namespace aurora {
namespace db {
class DbCellLib;
class DbView;
}
}  // namespace aurora

namespace aurora::ui {

class PropertyEditorWidget : public QWidget {
  Q_OBJECT

 public:
  explicit PropertyEditorWidget(QWidget* parent = nullptr);

  void showInstance(const db::DbView& view, db::DbId instanceId, const db::DbCellLib* lib);
  void showShape(const db::DbView& view, db::DbId shapeId);
  void showNet(const db::DbView& view, db::DbId netId);
  void clear();

 private:
  void addRow(const QString& label, const QString& value);
  void clearRows();

  QWidget*      content_{nullptr};
  QFormLayout*  form_{nullptr};
};

}  // namespace aurora::ui
