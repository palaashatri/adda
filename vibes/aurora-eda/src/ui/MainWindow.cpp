#include "ui/MainWindow.h"

#include "layout/LayDocument.h"
#include "layout/LayEditorController.h"
#include "layout/LayGdsWriter.h"
#include "layout/LayToolPolygon.h"
#include "layout/LayToolRect.h"
#include "layout/LayToolSelect.h"
#include "netlist/SpiceImporter.h"
#include "schematic/SchDocument.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchToolInstance.h"
#include "schematic/SchToolSelect.h"
#include "schematic/SchToolWire.h"
#include "sim/SimRunner.h"
#include "ui/CellBrowserDialog.h"
#include "ui/DrcResultsDialog.h"
#include "ui/LayerPaletteWidget.h"
#include "ui/LayoutViewWidget.h"
#include "ui/PropertyEditorWidget.h"
#include "ui/SchematicViewWidget.h"
#include "ui/SimSetupDialog.h"
#include "ui/WaveformViewWidget.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
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
  resize(1500, 950);

  // Create view widgets
  schView_  = new SchematicViewWidget(tabs_);
  layView_  = new LayoutViewWidget(tabs_);
  waveView_ = new WaveformViewWidget(tabs_);
  tabs_->addTab(schView_,  "Schematic");
  tabs_->addTab(layView_,  "Layout");
  tabs_->addTab(waveView_, "Waveforms");
  setCentralWidget(tabs_);

  connect(schView_, &SchematicViewWidget::coordinatesChanged, this, &MainWindow::onCoordinatesChanged);
  connect(layView_, &LayoutViewWidget::coordinatesChanged, this,   &MainWindow::onCoordinatesChanged);

  setupMenuBar();
  setupToolBar();
  setupToolToolBar();
  setupDocks();

  coordLabel_ = new QLabel("  x: 0.000  y: 0.000 ", this);
  coordLabel_->setMinimumWidth(200);
  statusBar()->addPermanentWidget(coordLabel_);
  statusBar()->showMessage("Ready");

  // Simulation runner
  simRunner_ = std::make_unique<sim::SimRunner>(
      std::filesystem::temp_directory_path() / "aurora-eda-sim");

  createDemoData();
  log("aurora-eda started  |  W=Wire  S=Select  I=Instance  R=Rect  Esc=Cancel  Del=Delete");
}

MainWindow::~MainWindow() = default;

void MainWindow::createDemoData() {
  auto& lib = app_.projects().workingLibrary();

  // Layers
  auto& diffLayer  = lib.createLayer("diff",   "drawing"); diffLayer.setColor("#3fbf7f"); diffLayer.setGdsMapping(22, 0);
  auto& polyLayer  = lib.createLayer("poly",   "drawing"); polyLayer.setColor("#e05050"); polyLayer.setGdsMapping(46, 0);
  auto& met1Layer  = lib.createLayer("metal1", "drawing"); met1Layer.setColor("#5080d0"); met1Layer.setGdsMapping(68, 0);

  auto& cell = lib.createCell("NMOS_demo");

  // Layout view
  auto& lv = cell.createView(db::DbViewType::Layout);
  using B = geom::GeomBox;
  (void)lv.createRect(diffLayer.id(), B{    0,     0,  3000,  4000});
  (void)lv.createRect(polyLayer.id(), B{ 1200,  -400,  1800,  4400});
  (void)lv.createRect(met1Layer.id(), B{  200,  3200,  2800,  4000});
  (void)lv.createRect(met1Layer.id(), B{  200,     0,  2800,   800});
  (void)lv.createText(diffLayer.id(), {-200, -200}, "NMOS_demo");

  layDoc_  = std::make_unique<layout::LayDocument>(lv);
  layCtrl_ = std::make_unique<layout::LayEditorController>(*layDoc_);
  layCtrl_->setActiveTool(std::make_unique<layout::LayToolRect>());
  layView_->setDocument(layDoc_.get(), &lib);
  layView_->setController(layCtrl_.get());

  connect(layerPalette_, &LayerPaletteWidget::layerSelectionChanged, [this](db::DbId id) {
    if (layCtrl_) layCtrl_->setActiveLayerId(id);
  });

  // Schematic view
  auto& sv  = cell.createView(db::DbViewType::Schematic);
  auto& vdd = sv.createNet("VDD");
  auto& gnd = sv.createNet("GND");
  auto& g   = sv.createNet("GATE");
  auto& d   = sv.createNet("DRAIN");

  schDoc_  = std::make_unique<schematic::SchDocument>(sv);
  schCtrl_ = std::make_unique<schematic::SchEditorController>(*schDoc_);
  schCtrl_->setActiveTool(std::make_unique<schematic::SchToolWire>());

  using P = geom::GeomPoint;
  (void)schDoc_->addWire(vdd.id(), {P{20000, 50000}, P{20000, 80000}});
  (void)schDoc_->addWire(gnd.id(), {P{20000,     0}, P{20000, 20000}});
  (void)schDoc_->addWire(g.id(),   {P{    0, 40000}, P{20000, 40000}});
  (void)schDoc_->addWire(d.id(),   {P{20000, 40000}, P{40000, 40000}});

  schView_->setDocument(schDoc_.get(), &lib);
  schView_->setController(schCtrl_.get());

  layerPalette_->setLibrary(&lib, &app_.tech());
  refreshLibraryBrowser();

  // Wire up simulation dialog
  simDialog_->setSimRunner(simRunner_.get());
  simDialog_->setCell(&lib, &cell);

  // Wire up DRC/LVS dialog
  drcDialog_->setLayoutView(&lv, &lib, &app_.tech());
  drcDialog_->setSchematicView(&sv, &lv, &lib);
  drcDialog_->setLayoutWidget(layView_);
}

static QAction* addMenuAction(QMenu* menu, const QString& text, const QKeySequence& shortcut,
                              QObject* receiver, const char* slot) {
  auto* act = menu->addAction(text);
  act->setShortcut(shortcut);
  QObject::connect(act, SIGNAL(triggered()), receiver, slot);
  return act;
}

void MainWindow::setupMenuBar() {
  // File
  auto* fileMenu = menuBar()->addMenu("&File");
  addMenuAction(fileMenu, "&New Project…",   QKeySequence::New,  this, SLOT(onNewProject()));
  addMenuAction(fileMenu, "&Open Project…",  QKeySequence::Open, this, SLOT(onOpenProject()));
  addMenuAction(fileMenu, "&Save Project",   QKeySequence::Save, this, SLOT(onSaveProject()));
  fileMenu->addSeparator();
  fileMenu->addAction("&Quit", qApp, SLOT(quit()), QKeySequence::Quit);

  // Edit (stub)
  auto* editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction("Undo")->setShortcut(QKeySequence::Undo);
  editMenu->addAction("Redo")->setShortcut(QKeySequence::Redo);

  // View
  auto* viewMenu = menuBar()->addMenu("&View");
  addMenuAction(viewMenu, "Zoom &In",  QKeySequence::ZoomIn,  this, SLOT(onZoomIn()));
  addMenuAction(viewMenu, "Zoom &Out", QKeySequence::ZoomOut, this, SLOT(onZoomOut()));
  {
    auto* fitAct = viewMenu->addAction("&Fit View");
    fitAct->setShortcut(Qt::Key_F);
    connect(fitAct, &QAction::triggered, this, &MainWindow::onZoomFit);
  }
  viewMenu->addSeparator();
  viewMenu->addAction("&Schematic",  this, [this] { tabs_->setCurrentIndex(0); })->setShortcut(Qt::Key_E);
  viewMenu->addAction("&Layout",     this, [this] { tabs_->setCurrentIndex(1); })->setShortcut(Qt::Key_L);
  viewMenu->addAction("&Waveforms",  this, [this] { tabs_->setCurrentIndex(2); })->setShortcut(Qt::Key_V);

  // Tools
  auto* toolMenu = menuBar()->addMenu("&Tools");
  {
    auto* a = toolMenu->addAction("&Select",        this, &MainWindow::onToolSelect);
    a->setShortcut(Qt::Key_S);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Wire",          this, &MainWindow::onToolWire);
    a->setShortcut(Qt::Key_W);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("Place &Instance",this, &MainWindow::onToolInstance);
    a->setShortcut(Qt::Key_I);
    a->setCheckable(true);
  }
  toolMenu->addSeparator();
  {
    auto* a = toolMenu->addAction("&Rectangle",     this, &MainWindow::onToolRect);
    a->setShortcut(Qt::Key_R);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Polygon",       this, &MainWindow::onToolPolygon);
    a->setShortcut(Qt::Key_P);
    a->setCheckable(true);
  }

  // Simulation
  auto* simMenu = menuBar()->addMenu("Si&mulation");
  {
    auto* a = simMenu->addAction("&Setup && Run…", this, &MainWindow::onSimSetup);
    a->setShortcut(Qt::CTRL | Qt::Key_R);
  }
  simMenu->addAction("View &Waveforms", this, [this] { tabs_->setCurrentIndex(2); });

  // Verification
  auto* verMenu = menuBar()->addMenu("&Verification");
  {
    auto* a = verMenu->addAction("&DRC / LVS Results…", this, &MainWindow::onDrcLvs);
    a->setShortcut(Qt::CTRL | Qt::Key_D);
  }

  // Import / Export
  auto* impMenu = menuBar()->addMenu("&Import/Export");
  impMenu->addAction("Import &SPICE Netlist…", this, &MainWindow::onImportSpice);
  impMenu->addAction("Export &GDS II…",        this, &MainWindow::onExportGds);

  // Help
  auto* helpMenu = menuBar()->addMenu("&Help");
  helpMenu->addAction("&About aurora-eda", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
  auto* toolbar = addToolBar("Main");
  toolbar->setMovable(false);

  auto* newAct  = toolbar->addAction("New");   newAct->setToolTip("New Project");
  connect(newAct, &QAction::triggered, this, &MainWindow::onNewProject);

  auto* openAct = toolbar->addAction("Open");  openAct->setToolTip("Open Project");
  connect(openAct, &QAction::triggered, this, &MainWindow::onOpenProject);

  auto* saveAct = toolbar->addAction("Save");  saveAct->setToolTip("Save Project");
  connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveProject);

  toolbar->addSeparator();

  auto* zoomIn  = toolbar->addAction("⊕"); zoomIn->setToolTip("Zoom In");
  connect(zoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);
  auto* zoomOut = toolbar->addAction("⊖"); zoomOut->setToolTip("Zoom Out");
  connect(zoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
  auto* fitAct  = toolbar->addAction("Fit"); fitAct->setToolTip("Fit View (F)");
  connect(fitAct, &QAction::triggered, this, &MainWindow::onZoomFit);

  toolbar->addSeparator();

  auto* schAct  = toolbar->addAction("Sch"); schAct->setToolTip("Switch to Schematic (E)");
  connect(schAct, &QAction::triggered, this, [this] { tabs_->setCurrentIndex(0); });
  auto* layAct  = toolbar->addAction("Lay"); layAct->setToolTip("Switch to Layout (L)");
  connect(layAct, &QAction::triggered, this, [this] { tabs_->setCurrentIndex(1); });

  toolbar->addSeparator();

  auto* simAct  = toolbar->addAction("▶ Sim"); simAct->setToolTip("Simulation Setup & Run");
  connect(simAct, &QAction::triggered, this, &MainWindow::onSimSetup);
  auto* drcAct  = toolbar->addAction("✓ DRC"); drcAct->setToolTip("DRC / LVS Results");
  connect(drcAct, &QAction::triggered, this, &MainWindow::onDrcLvs);
}

void MainWindow::setupToolToolBar() {
  auto* toolbar = addToolBar("Tools");
  toolbar->setMovable(false);
  toolbar->setWindowTitle("Tool Palette");

  auto makeBtn = [&](const QString& label, const QString& tip, auto slot) {
    auto* a = toolbar->addAction(label);
    a->setToolTip(tip);
    a->setCheckable(true);
    connect(a, &QAction::triggered, this, slot);
    return a;
  };

  makeBtn("S", "Select (S)",         &MainWindow::onToolSelect);
  makeBtn("W", "Wire (W)",           &MainWindow::onToolWire);
  makeBtn("I", "Place Instance (I)", &MainWindow::onToolInstance);
  toolbar->addSeparator();
  makeBtn("R", "Rectangle (R)",      &MainWindow::onToolRect);
  makeBtn("P", "Polygon (P)",        &MainWindow::onToolPolygon);
}

void MainWindow::setupDocks() {
  // Library browser
  auto* libContainer = new QWidget(this);
  auto* libLayout = new QVBoxLayout(libContainer);
  libLayout->setContentsMargins(0, 0, 0, 0);
  libraryTree_ = new QTreeWidget(libContainer);
  libraryTree_->setHeaderLabel("Library / Cell");
  libraryTree_->setMinimumWidth(180);
  connect(libraryTree_, &QTreeWidget::itemDoubleClicked, this,
          &MainWindow::onLibraryItemDoubleClicked);
  libLayout->addWidget(libraryTree_);
  auto* libraryDock = new QDockWidget("Library Browser", this);
  libraryDock->setWidget(libContainer);
  addDockWidget(Qt::LeftDockWidgetArea, libraryDock);

  // Properties
  propWidget_ = new PropertyEditorWidget(this);
  auto* propDock = new QDockWidget("Properties", this);
  propDock->setWidget(propWidget_);
  addDockWidget(Qt::RightDockWidgetArea, propDock);

  // Layers
  auto* layersDock = new QDockWidget("Layers", this);
  layerPalette_ = new LayerPaletteWidget(layersDock);
  layersDock->setWidget(layerPalette_);
  addDockWidget(Qt::RightDockWidgetArea, layersDock);
  connect(layerPalette_, &LayerPaletteWidget::layerVisibilityChanged, layView_,
          &LayoutViewWidget::setLayerVisible);

  // Log pane
  logPane_ = new QPlainTextEdit(this);
  logPane_->setReadOnly(true);
  logPane_->setMaximumBlockCount(3000);
  auto* logDock = new QDockWidget("Log", this);
  logDock->setWidget(logPane_);
  addDockWidget(Qt::BottomDockWidgetArea, logDock);

  // Create dialogs (not shown yet)
  simDialog_ = new SimSetupDialog(this);
  connect(simDialog_, &SimSetupDialog::simulationFinished, this, &MainWindow::onSimFinished);

  drcDialog_ = new DrcResultsDialog(this);
}

void MainWindow::refreshLibraryBrowser() {
  if (!libraryTree_) return;
  libraryTree_->clear();
  const auto& worklib = app_.projects().workingLibrary();
  auto* libItem = new QTreeWidgetItem(libraryTree_, {QString::fromStdString(worklib.name())});
  libItem->setExpanded(true);

  for (const auto id : worklib.cellIds()) {
    const auto* cell = worklib.findCellById(id);
    if (!cell) continue;
    auto* cellItem = new QTreeWidgetItem(libItem, {QString::fromStdString(cell->name())});
    cellItem->setData(0, Qt::UserRole, static_cast<qlonglong>(id));
    for (const auto vid : cell->viewIds()) {
      const auto* view = cell->findViewById(vid);
      if (!view) continue;
      auto* vitem = new QTreeWidgetItem(cellItem,
          {QString::fromStdString(std::string(db::toString(view->type())))});
      vitem->setData(0, Qt::UserRole + 1, static_cast<int>(view->type()));
    }
  }
  libraryTree_->expandAll();
}

void MainWindow::log(const QString& message) {
  if (logPane_) logPane_->appendPlainText(message);
}

// ─── Tool actions ─────────────────────────────────────────────────────────────

void MainWindow::onToolSelect() {
  const int tab = tabs_->currentIndex();
  if (tab == 0 && schCtrl_) {
    schCtrl_->setActiveTool(std::make_unique<schematic::SchToolSelect>());
    statusBar()->showMessage("Schematic Select tool active — click to select, Delete to remove", 4000);
  } else if (tab == 1 && layCtrl_) {
    layCtrl_->setActiveTool(std::make_unique<layout::LayToolSelect>());
    statusBar()->showMessage("Layout Select tool active — drag to select, Delete to remove", 4000);
  }
}

void MainWindow::onToolWire() {
  if (schCtrl_) {
    tabs_->setCurrentIndex(0);
    schCtrl_->setActiveTool(std::make_unique<schematic::SchToolWire>());
    statusBar()->showMessage("Wire tool active — click to start, click to add segment, Esc to finish", 4000);
  }
}

void MainWindow::onToolInstance() {
  if (!schCtrl_) return;
  tabs_->setCurrentIndex(0);
  // Ask user which cell to place
  const auto& lib = app_.projects().workingLibrary();
  QStringList cells;
  for (const auto id : lib.cellIds())
    if (const auto* c = lib.findCellById(id)) cells << QString::fromStdString(c->name());
  if (cells.isEmpty()) { statusBar()->showMessage("No cells in library.", 3000); return; }
  bool ok = false;
  const auto chosen = QInputDialog::getItem(this, "Place Instance", "Select cell:", cells, 0, false, &ok);
  if (!ok) return;
  const auto* cell = lib.findCell(chosen.toStdString());
  if (!cell) return;
  schCtrl_->setActiveTool(std::make_unique<schematic::SchToolInstance>(cell->id()));
  statusBar()->showMessage("Instance tool active — click to place " + chosen, 4000);
}

void MainWindow::onToolRect() {
  if (layCtrl_) {
    tabs_->setCurrentIndex(1);
    layCtrl_->setActiveTool(std::make_unique<layout::LayToolRect>());
    statusBar()->showMessage("Rectangle tool — click and drag to draw a rectangle", 4000);
  }
}

void MainWindow::onToolPolygon() {
  if (layCtrl_) {
    tabs_->setCurrentIndex(1);
    layCtrl_->setActiveTool(std::make_unique<layout::LayToolPolygon>());
    statusBar()->showMessage("Polygon tool — click to add vertices, Enter to close, Esc to cancel", 4000);
  }
}

// ─── Simulation ───────────────────────────────────────────────────────────────

void MainWindow::onSimSetup() {
  simDialog_->show();
  simDialog_->raise();
}

void MainWindow::onSimFinished(const sim::SimResult& result) {
  waveView_->clearTraces();
  if (!result.waveforms.empty()) {
    const QList<QColor> palette{
      QColor("#00ccff"), QColor("#ff8844"), QColor("#44ff88"),
      QColor("#ff44cc"), QColor("#ffcc00"), QColor("#cc44ff")};
    int ci = 0;
    for (const auto& wv : result.waveforms) {
      waveView_->addTrace(wv.name, wv.x, wv.y, palette[ci++ % palette.size()]);
    }
    tabs_->setCurrentIndex(2);
    log("Waveforms loaded: " + QString::number(result.waveforms.size()) + " traces");
  } else if (!result.dcOperatingPoint.empty()) {
    // Show DC op-point as a bar/column chart (single values)
    std::vector<double> x, y;
    std::vector<std::string> names;
    int i = 0;
    for (const auto& [k, v] : result.dcOperatingPoint) {
      x.push_back(i++);
      y.push_back(v);
      names.push_back(k);
    }
    waveView_->setAxisLabels("Node", "Voltage / Value");
    waveView_->addTrace("Operating Point", x, y, QColor("#00ccff"));
    tabs_->setCurrentIndex(2);
    log("DC operating point loaded — " + QString::number(x.size()) + " nodes");
  }
  if (!result.success)
    log("Simulation failed: " + QString::fromStdString(result.errorMessage));
}

// ─── Verification ─────────────────────────────────────────────────────────────

void MainWindow::onDrcLvs() {
  drcDialog_->show();
  drcDialog_->raise();
}

// ─── Import / Export ──────────────────────────────────────────────────────────

void MainWindow::onImportSpice() {
  const auto path = QFileDialog::getOpenFileName(this, "Import SPICE Netlist",
      QDir::homePath(), "SPICE Files (*.sp *.cir *.spi *.net);;All Files (*)");
  if (path.isEmpty()) return;
  netlist::SpiceImporter importer;
  if (importer.importFile(path.toStdString(), app_.projects().workingLibrary())) {
    refreshLibraryBrowser();
    log("Imported SPICE netlist: " + path);
    statusBar()->showMessage("SPICE import successful", 4000);
  } else {
    QMessageBox::warning(this, "Import Error",
        QString::fromStdString(importer.lastError()));
    log("SPICE import failed: " + QString::fromStdString(importer.lastError()));
  }
}

void MainWindow::onExportGds() {
  if (!layDoc_) { statusBar()->showMessage("No layout loaded", 3000); return; }
  const auto path = QFileDialog::getSaveFileName(this, "Export GDS II",
      QDir::homePath(), "GDS II (*.gds);;All Files (*)");
  if (path.isEmpty()) return;
  layout::LayGdsWriter writer;
  if (writer.write(app_.projects().workingLibrary(), path.toStdString())) {
    log("GDS II exported: " + path);
    statusBar()->showMessage("GDS exported", 4000);
  } else {
    QMessageBox::warning(this, "Export Error", "Failed to write GDS II file.");
  }
}

// ─── Library browser ──────────────────────────────────────────────────────────

void MainWindow::onLibraryItemDoubleClicked(QTreeWidgetItem* item, int) {
  if (!item) return;
  const auto cellId = static_cast<db::DbId>(item->data(0, Qt::UserRole).toLongLong());
  if (cellId == 0) return;  // library root or view item
  // Double-click on a cell → switch to instance placement tool in schematic
  if (schCtrl_) {
    tabs_->setCurrentIndex(0);
    schCtrl_->setActiveTool(std::make_unique<schematic::SchToolInstance>(cellId));
    const auto& lib = app_.projects().workingLibrary();
    const auto* cell = lib.findCellById(cellId);
    const auto name = cell ? QString::fromStdString(cell->name()) : "cell";
    statusBar()->showMessage("Placing instance of '" + name + "' — click on schematic", 5000);
    log("Instance tool: placing " + name);
  }
}

// ─── Standard actions ─────────────────────────────────────────────────────────

void MainWindow::onNewProject() {
  const auto dir = QFileDialog::getExistingDirectory(this, "Select New Project Directory");
  if (dir.isEmpty()) return;
  if (app_.projects().createProject(dir.toStdString())) {
    setWindowTitle(QString("aurora-eda  —  %1").arg(QFileInfo(dir).fileName()));
    refreshLibraryBrowser();
    log("Created project: " + dir);
    statusBar()->showMessage("Project created: " + dir, 5000);
  } else {
    QMessageBox::warning(this, "aurora-eda", "Failed to create project.");
  }
}

void MainWindow::onOpenProject() {
  const auto dir = QFileDialog::getExistingDirectory(this, "Open Project Directory");
  if (dir.isEmpty()) return;
  if (app_.projects().openProject(dir.toStdString())) {
    setWindowTitle(QString("aurora-eda  —  %1").arg(QFileInfo(dir).fileName()));
    refreshLibraryBrowser();
    log("Opened project: " + dir);
    statusBar()->showMessage("Project opened: " + dir, 5000);
  } else {
    QMessageBox::warning(this, "aurora-eda", "Failed to open project.");
  }
}

void MainWindow::onSaveProject() {
  if (!app_.projects().hasOpenProject()) {
    statusBar()->showMessage("No open project to save", 3000); return;
  }
  if (app_.projects().saveProject()) {
    log("Project saved");
    statusBar()->showMessage("Project saved", 3000);
  } else {
    QMessageBox::warning(this, "aurora-eda", "Failed to save project.");
  }
}

static void sendWheelStep(QWidget* w, int delta) {
  if (!w) return;
  const auto center = w->rect().center();
  QWheelEvent fake(QPointF(center), w->mapToGlobal(QPointF(center)), QPoint{},
                   QPoint{0, delta}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
  QApplication::sendEvent(w, &fake);
}

void MainWindow::onZoomIn() {
  const int idx = tabs_->currentIndex();
  QWidget* w = (idx == 0) ? static_cast<QWidget*>(schView_) :
               (idx == 1) ? static_cast<QWidget*>(layView_) : nullptr;
  sendWheelStep(w, 120);
}

void MainWindow::onZoomOut() {
  const int idx = tabs_->currentIndex();
  QWidget* w = (idx == 0) ? static_cast<QWidget*>(schView_) :
               (idx == 1) ? static_cast<QWidget*>(layView_) : nullptr;
  sendWheelStep(w, -120);
}

void MainWindow::onZoomFit() {
  const int idx = tabs_->currentIndex();
  if (idx == 0 && schView_)  schView_->fitView();
  if (idx == 1 && layView_)  layView_->fitView();
  if (idx == 2 && waveView_) waveView_->fitView();
}

void MainWindow::onAbout() {
  QMessageBox::about(this, "About aurora-eda",
      QString("<b>aurora-eda</b> %1<br><br>"
              "Open-source analog/custom IC design environment.<br>"
              "Virtuoso-style workflows without proprietary dependencies.<br><br>"
              "<b>Keyboard shortcuts:</b><br>"
              "W=Wire  S=Select  I=Instance  R=Rect  P=Polygon<br>"
              "F=Fit  E=Schematic  L=Layout  V=Waveforms<br>"
              "Ctrl+R=Run Sim  Ctrl+D=DRC/LVS<br>"
              "Del=Delete selected  Esc=Cancel tool<br><br>"
              "Licensed for academic use.")
          .arg(QString::fromStdString(std::string(aurora::core::CoreApp::version()))));
}

void MainWindow::onCoordinatesChanged(QPointF scenePt) {
  if (coordLabel_)
    coordLabel_->setText(QString("  x: %1  y: %2 ")
        .arg(scenePt.x(), 0, 'f', 3).arg(scenePt.y(), 0, 'f', 3));
}

}  // namespace aurora::ui
