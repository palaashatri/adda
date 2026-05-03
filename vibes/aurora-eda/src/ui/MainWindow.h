#pragma once

#include <QMainWindow>

class QPlainTextEdit;
class QTabWidget;

namespace aurora::ui {

class MainWindow : public QMainWindow {
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private:
  void setupDocks();

  QTabWidget* tabs_{nullptr};
  QPlainTextEdit* logPane_{nullptr};
};

}  // namespace aurora::ui
