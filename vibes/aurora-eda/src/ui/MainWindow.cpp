#include "ui/MainWindow.h"

#include "layout/LayDocument.h"
#include "layout/LayEditorController.h"
#include "layout/LayToolRect.h"
#include "schematic/SchDocument.h"
#include "ui/LayerPaletteWidget.h"
#include "ui/LayoutViewWidget.h"
#include "ui/PropertyEditorWidget.h"
#include "ui/SchematicViewWidget.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWheelEvent>

namespace aurora::ui {

MainWindow::MainWindow(aurora::core::CoreApp& app, QWidget* parent)
    : QMainWindow(parent), app_(app), tabs_(new QTabWidget(this)) {
  setWindowTitle(QString("aurora-eda  —  %1").arg(
      QString::fromStdString(std::string(aurora::core::CoreApp::applicationName()))));
  resize(1400, 900);

  schView_ = new SchematicViewWidget(tabs_);
  layView_ = new LayoutViewWidget(tabs_);
  tabs_->addTab(schView_, "Schematic");
  tabs_->addTab(layView_, "Layout");
  setCentralWidget(tabs_);

  connect(schView_, &SchematicViewWidget::coordinatesChanged, this,
          &MainWindow::onCoordinatesChanged);
  connect(layView_, &LayoutViewWidget::coordinatesChanged, this,
          &MainWindow::onCoordinatesChanged);

  setupMenuBar();
  setupToolBar();
  setupDocks();

  coordLabel_ = new QLabel("  x: 0.000  y: 0.000 ", this);
  coordLabel_->setMinimumWidth(200);
  statusBar()->addPermanentWidget(coordLabel_);
  statusBar()->showMessage("Ready");

  createDemoData();
  log("aurora-eda started");
}

void MainWindow::createDemoData() {
  auto& lib = app_.projects().workingLibrary();

  auto& diffLayer  = lib.createLayer("diff",   "drawing");
  diffLayer.setColor("#3fbf7f");
  diffLayer.setGdsMapping(22, 0);

  auto& polyLayer  = lib.createLayer("poly",   "drawing");
  polyLayer.setColor("#e05050");
  polyLayer.setGdsMapping(46, 0);

  auto& met1Layer  = lib.createLayer("metal1", "drawing");
  met1Layer.setColor("#5080d0");
  met1Layer.setGdsMapping(68, 0);

  auto& cell = lib.createCell("NMOS_demo");

  // Layout view — single NMOS finger (dbu = nm, 1 µm = 1000 dbu)
  auto& lv = cell.createView(db::DbViewType::Layout);
  using B = geom::GeomBox;
  const db::DbId diffId = diffLayer.id();
  const db::DbId polyId = polyLayer.id();
  const db::DbId met1Id = met1Layer.id();
  (void)lv.createRect(diffId, B{    0,     0,  3000,  4000});
  (void)lv.createRect(polyId, B{ 1200,  -400,  1800,  4400});
  (void)lv.createRect(met1Id, B{  200,  3200,  2800,  4000});
  (void)lv.createRect(met1Id, B{  200,     0,  2800,   800});
  (void)lv.createText(diffId, {-200, -200}, "NMOS_demo");

  layDoc_ = std::make_unique<layout::LayDocument>(lv);
  layCtrl_ = std::make_unique<layout::LayEditorController>(*layDoc_);
  layCtrl_->setActiveTool(std::make_unique<layout::LayToolRect>());
  
  layView_->setDocument(layDoc_.get(), &lib);
  layView_->setController(layCtrl_.get());

  connect(layerPalette_, &LayerPaletteWidget::layerSelectionChanged, [this](db::DbId id) {
    if (layCtrl_) layCtrl_->setActiveLayerId(id);
  });

  // Schematic view — stub wires forming a T/H pattern
  auto& sv  = cell.createView(db::DbViewType::Schematic);
  auto& vdd = sv.createNet("VDD");
  auto& gnd = sv.createNet("GND");
  auto& g   = sv.createNet("GATE");
  auto& d   = sv.createNet("DRAIN");

  schDoc_ = std::make_unique<schematic::SchDocument>(sv);
  using P = geom::GeomPoint;
  (void)schDoc_->addWire(vdd.id(),  {P{20000, 50000}, P{20000, 80000}});
  (void)schDoc_->addWire(gnd.id(),  {P{20000,     0}, P{20000, 20000}});
  (void)schDoc_->addWire(g.id(),    {P{    0, 40000}, P{20000, 40000}});
  (void)schDoc_->addWire(d.id(),    {P{20000, 40000}, P{40000, 40000}});

  schView_->setDocument(schDoc_.get(), &lib);
  layerPalette_->setLibrary(&lib, &app_.tech());
  refreshLibraryBrowser();
}

static QAction* addMenuAction(QMenu* menu, const QString& text, const QKeySequence& shortcut,
                              QObject* receiver, const char* slot) {
  auto* act = menu->addAction(text);
  act->setShortcut(shortcut);
  QObject::connect(act, SIGNAL(triggered()), receiver, slot);
  return act;
}

void MainWindow::setupMenuBar() {
  auto* fileMenu = menuBar()->addMenu("&File");
  addMenuAction(fileMenu, "&New Project…", QKeySequence::New, this,
                SLOT(onNewProject()));
  addMenuAction(fileMenu, "&Open Project…", QKeySequence::Open, this,
                SLOT(onOpenProject()));
  addMenuAction(fileMenu, "&Save Project", QKeySequence::Save, this,
                SLOT(onSaveProject()));
  fileMenu->addSeparator();
  addMenuAction(fileMenu, "&Quit", QKeySequence::Quit, qApp, SLOT(quit()));

  auto* viewMenu = menuBar()->addMenu("&View");
  addMenuAction(viewMenu, "Zoom &In", QKeySequence::ZoomIn, this, SLOT(onZoomIn()));
  addMenuAction(viewMenu, "Zoom &Out", QKeySequence::ZoomOut, this, SLOT(onZoomOut()));
  {
    auto* fitAct = viewMenu->addAction("&Fit View");
    fitAct->setShortcut(QKeySequence(Qt::Key_F));
    connect(fitAct, &QAction::triggered, this, &MainWindow::onZoomFit);
  }
  viewMenu->addSeparator();
  viewMenu->addAction("&Schematic", this, [this] { tabs_->setCurrentIndex(0); });
  viewMenu->addAction("&Layout", this, [this] { tabs_->setCurrentIndex(1); });

  auto* helpMenu = menuBar()->addMenu("&Help");
  helpMenu->addAction("&About aurora-eda", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
  auto* toolbar = addToolBar("Main");
  toolbar->setMovable(false);

  auto* newAct = toolbar->addAction("New");
  newAct->setToolTip("New Project");
  connect(newAct, &QAction::triggered, this, &MainWindow::onNewProject);

  auto* openAct = toolbar->addAction("Open");
  openAct->setToolTip("Open Project");
  connect(openAct, &QAction::triggered, this, &MainWindow::onOpenProject);

  auto* saveAct = toolbar->addAction("Save");
  saveAct->setToolTip("Save Project");
  connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveProject);

  toolbar->addSeparator();

  auto* zoomInAct = toolbar->addAction("⊕");
  zoomInAct->setToolTip("Zoom In");
  connect(zoomInAct, &QAction::triggered, this, &MainWindow::onZoomIn);

  auto* zoomOutAct = toolbar->addAction("⊖");
  zoomOutAct->setToolTip("Zoom Out");
  connect(zoomOutAct, &QAction::triggered, this, &MainWindow::onZoomOut);

  auto* fitAct = toolbar->addAction("Fit");
  fitAct->setToolTip("Fit View (F)");
  connect(fitAct, &QAction::triggered, this, &MainWindow::onZoomFit);

  toolbar->addSeparator();

  auto* schAct = toolbar->addAction("Sch");
  schAct->setToolTip("Switch to Schematic");
  connect(schAct, &QAction::triggered, this, [this] { tabs_->setCurrentIndex(0); });

  auto* layAct = toolbar->addAction("Lay");
  layAct->setToolTip("Switch to Layout");
  connect(layAct, &QAction::triggered, this, [this] { tabs_->setCurrentIndex(1); });
}

void MainWindow::setupDocks() {
  auto* libraryContainer = new QWidget(this);
  auto* libLayout = new QVBoxLayout(libraryContainer);
  libLayout->setContentsMargins(0, 0, 0, 0);
  libraryTree_ = new QTreeWidget(libraryContainer);
  libraryTree_->setHeaderLabel("Library / Cell");
  libraryTree_->setMinimumWidth(180);
  libLayout->addWidget(libraryTree_);
  auto* libraryDock = new QDockWidget("Library Browser", this);
  libraryDock->setWidget(libraryContainer);
  addDockWidget(Qt::LeftDockWidgetArea, libraryDock);

  auto* propertiesDock = new QDockWidget("Properties", this);
  propertiesDock->setWidget(new PropertyEditorWidget(propertiesDock));
  addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

  auto* layersDock = new QDockWidget("Layers", this);
  layerPalette_ = new LayerPaletteWidget(layersDock);
  layersDock->setWidget(layerPalette_);
  addDockWidget(Qt::RightDockWidgetArea, layersDock);

  connect(layerPalette_, &LayerPaletteWidget::layerVisibilityChanged, layView_,
          &LayoutViewWidget::setLayerVisible);

  logPane_ = new QPlainTextEdit(this);
  logPane_->setReadOnly(true);
  logPane_->setMaximumBlockCount(2000);
  auto* logDock = new QDockWidget("Log", this);
  logDock->setWidget(logPane_);
  addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::refreshLibraryBrowser() {
  if (libraryTree_ == nullptr) {
    return;
  }
  libraryTree_->clear();

  const auto& worklib = app_.projects().workingLibrary();
  auto* libItem = new QTreeWidgetItem(libraryTree_, {QString::fromStdString(worklib.name())});
  libItem->setExpanded(true);

  for (const auto id : worklib.cellIds()) {
    const auto* cell = worklib.findCellById(id);
    if (cell == nullptr) {
      continue;
    }
    auto* cellItem = new QTreeWidgetItem(libItem, {QString::fromStdString(cell->name())});
    for (const auto vid : cell->viewIds()) {
      const auto* view = cell->findViewById(vid);
      if (view == nullptr) {
        continue;
      }
      (void)new QTreeWidgetItem(cellItem,
                                {QString::fromStdString(std::string(db::toString(view->type())))});
    }
  }

  libraryTree_->expandAll();
}

void MainWindow::log(const QString& message) {
  if (logPane_ != nullptr) {
    logPane_->appendPlainText(message);
  }
}

void MainWindow::onNewProject() {
  const auto dir =
      QFileDialog::getExistingDirectory(this, "Select New Project Directory", QDir::homePath());
  if (dir.isEmpty()) {
    return;
  }
  if (app_.projects().createProject(dir.toStdString())) {
    setWindowTitle(
        QString("aurora-eda  —  %1").arg(QFileInfo(dir).fileName()));
    refreshLibraryBrowser();
    log(QString("Created project: %1").arg(dir));
    statusBar()->showMessage(QString("Project created: %1").arg(dir), 5000);
  } else {
    log("Error: failed to create project");
    QMessageBox::warning(this, "aurora-eda", "Failed to create project in the selected directory.");
  }
}

void MainWindow::onOpenProject() {
  const auto dir =
      QFileDialog::getExistingDirectory(this, "Open Project Directory", QDir::homePath());
  if (dir.isEmpty()) {
    return;
  }
  if (app_.projects().openProject(dir.toStdString())) {
    setWindowTitle(
        QString("aurora-eda  —  %1").arg(QFileInfo(dir).fileName()));
    refreshLibraryBrowser();
    log(QString("Opened project: %1").arg(dir));
    statusBar()->showMessage(QString("Project opened: %1").arg(dir), 5000);
  } else {
    log("Error: failed to open project");
    QMessageBox::warning(this, "aurora-eda", "Failed to open the selected directory as a project.");
  }
}

void MainWindow::onSaveProject() {
  if (!app_.projects().hasOpenProject()) {
    statusBar()->showMessage("No open project to save", 3000);
    return;
  }
  if (app_.projects().saveProject()) {
    log("Project saved");
    statusBar()->showMessage("Project saved", 3000);
  } else {
    log("Error: failed to save project");
    QMessageBox::warning(this, "aurora-eda", "Failed to save the project.");
  }
}

static void sendWheelStep(QWidget* widget, int delta) {
  if (widget == nullptr) {
    return;
  }
  const auto center = widget->rect().center();
  QWheelEvent fake(QPointF(center), widget->mapToGlobal(QPointF(center)), QPoint{},
                   QPoint{0, delta}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QApplication::sendEvent(widget, &fake);
}

void MainWindow::onZoomIn() {
  const auto idx = tabs_->currentIndex();
  sendWheelStep(idx == 0 ? static_cast<QWidget*>(schView_) : static_cast<QWidget*>(layView_),
                120);
}

void MainWindow::onZoomOut() {
  const auto idx = tabs_->currentIndex();
  sendWheelStep(idx == 0 ? static_cast<QWidget*>(schView_) : static_cast<QWidget*>(layView_),
                -120);
}

void MainWindow::onZoomFit() {
  if (tabs_->currentIndex() == 0 && schView_ != nullptr) {
    schView_->fitView();
  } else if (tabs_->currentIndex() == 1 && layView_ != nullptr) {
    layView_->fitView();
  }
}

void MainWindow::onAbout() {
  QMessageBox::about(
      this, "About aurora-eda",
      QString("<b>aurora-eda</b> %1<br><br>"
              "An open-source, extensible analog/custom IC design environment "
              "approximating Cadence Virtuoso-style workflows.<br><br>"
              "Licensed for academic use.")
          .arg(QString::fromStdString(std::string(aurora::core::CoreApp::version()))));
}

void MainWindow::onCoordinatesChanged(QPointF scenePt) {
  if (coordLabel_ != nullptr) {
    coordLabel_->setText(QString("  x: %1  y: %2 ")
                             .arg(scenePt.x(), 0, 'f', 3)
                             .arg(scenePt.y(), 0, 'f', 3));
  }
}

}  // namespace aurora::ui
