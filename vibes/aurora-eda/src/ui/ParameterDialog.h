#pragma once

#include "db/DbTypes.h"

#include <QDialog>
#include <QString>

class QTableWidget;

namespace aurora::db { class DbInstance; }

namespace aurora::ui {

class ParameterDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ParameterDialog(db::DbInstance& inst, QWidget* parent = nullptr);

 private slots:
  void onAdd();
  void onRemove();

 private:
  db::DbInstance& inst_;
  QTableWidget* table_{nullptr};
};

}  // namespace aurora::ui
