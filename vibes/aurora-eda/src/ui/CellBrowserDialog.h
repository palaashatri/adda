#pragma once

#include "db/DbTypes.h"

#include <QDialog>

class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeWidget;

namespace aurora {
namespace db {
class DbCellLib;
}
}  // namespace aurora

namespace aurora::ui {

class CellBrowserDialog : public QDialog {
  Q_OBJECT

 public:
  explicit CellBrowserDialog(QWidget* parent = nullptr);

  void setLibrary(const db::DbCellLib* lib);

  [[nodiscard]] db::DbId selectedCellId() const { return selectedCellId_; }
  [[nodiscard]] db::DbViewType selectedViewType() const { return selectedViewType_; }

 private slots:
  void onCellSelectionChanged();
  void onViewDoubleClicked();
  void onNewCell();

 private:
  QTreeWidget*    cellTree_{nullptr};
  QListWidget*    viewList_{nullptr};
  QPushButton*    openBtn_{nullptr};
  QPushButton*    newCellBtn_{nullptr};

  const db::DbCellLib* lib_{nullptr};
  db::DbId         selectedCellId_{db::kInvalidId};
  db::DbViewType   selectedViewType_{db::DbViewType::Schematic};
};

}  // namespace aurora::ui
