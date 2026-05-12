#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;
class QListWidget;

namespace aurora::ui {

class ExpressionDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ExpressionDialog(const QStringList& traceNames, QWidget* parent = nullptr);

  [[nodiscard]] QString expression() const;
  [[nodiscard]] QString resultName() const;

 private slots:
  void onAddOp(const QString& op);
  void onClear();

 private:
  QComboBox* traceCombo_{nullptr};
  QLineEdit* exprEdit_{nullptr};
  QLineEdit* resultEdit_{nullptr};
  QListWidget* traceList_{nullptr};
};

}  // namespace aurora::ui
