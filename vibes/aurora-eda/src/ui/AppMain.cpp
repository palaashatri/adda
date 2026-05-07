#include "core/CoreApp.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("aurora-eda");
  QCoreApplication::setOrganizationName("aurora-eda");

  aurora::core::CoreApp coreApp;
  (void)coreApp.initialize();

  aurora::ui::MainWindow window(coreApp);
  window.show();

  const int result = QApplication::exec();
  coreApp.shutdown();
  return result;
}
