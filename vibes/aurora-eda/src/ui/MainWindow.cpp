#include "ui/MainWindow.h"

#include "ui/LayerPaletteWidget.h"
#include "ui/LayoutViewWidget.h"
#include "ui/PropertyEditorWidget.h"
#include "ui/SchematicViewWidget.h"

#include <QDockWidget>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTabWidget>

namespace aurora::ui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), tabs_(new QTabWidget(this)) {
  setWindowTitle("aurora-eda");
  resize(1280, 820);

  tabs_->addTab(new SchematicViewWidget(tabs_), "Schematic");
  tabs_->addTab(new LayoutViewWidget(tabs_), "Layout");
  setCentralWidget(tabs_);

  setupDocks();
  statusBar()->showMessage("Ready");
}

void MainWindow::setupDocks() {
  auto* library = new QListWidget(this);
  library->addItem("worklib");
  auto* libraryDock = new QDockWidget("Library Browser", this);
  libraryDock->setWidget(library);
  addDockWidget(Qt::LeftDockWidgetArea, libraryDock);

  auto* propertiesDock = new QDockWidget("Properties", this);
  propertiesDock->setWidget(new PropertyEditorWidget(propertiesDock));
  addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

  auto* layersDock = new QDockWidget("Layers", this);
  layersDock->setWidget(new LayerPaletteWidget(layersDock));
  addDockWidget(Qt::RightDockWidgetArea, layersDock);

  logPane_ = new QPlainTextEdit(this);
  logPane_->setReadOnly(true);
  logPane_->appendPlainText("aurora-eda log initialized");
  auto* logDock = new QDockWidget("Log", this);
  logDock->setWidget(logPane_);
  addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

}  // namespace aurora::ui
