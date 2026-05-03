#include "ui/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("aurora-eda");
  QCoreApplication::setOrganizationName("aurora-eda");

  aurora::ui::MainWindow window;
  window.show();

  return QApplication::exec();
}
