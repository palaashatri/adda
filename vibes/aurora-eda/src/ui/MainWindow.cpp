#include "ui/MainWindow.h"

#include "layout/LayDocument.h"
#include "layout/LayEditorController.h"
#include "pdk/PcellRegistry.h"
#include "layout/LayGdsReader.h"
#include "layout/LayDefReader.h"
#include "layout/LayDefWriter.h"
#include "layout/LayGdsWriter.h"
#include "layout/LayLefWriter.h"
#include "drc_lvs/DrcEngine.h"
#include "drc_lvs/DrcViolation.h"
#include "netlist/VerilogGenerator.h"
#include "layout/LayToolGuardRing.h"
#include "layout/LayToolPath.h"
#include "layout/LayToolPolygon.h"
#include "layout/LayToolRect.h"
#include "layout/LayToolRuler.h"
#include "layout/LayToolSelect.h"
#include "layout/LayToolStretch.h"
#include "layout/LayToolVia.h"
#include "layout/LayToolViaArray.h"
#include "netlist/SpiceImporter.h"
#include "schematic/SchDocument.h"
#include "schematic/SchEditorController.h"
#include "schematic/SchToolInstance.h"
#include "schematic/SchToolBusRip.h"
#include "schematic/SchToolLabel.h"
#include "schematic/SchToolProbe.h"
#include "schematic/SchToolSelect.h"
#include "schematic/SchToolStimulus.h"
#include "schematic/SchToolSymbolPin.h"
#include "schematic/SchToolWire.h"
#include "sim/SimRunner.h"
#include "ui/CellBrowserDialog.h"
#include "ui/DrcResultsDialog.h"
#include "ui/ExpressionDialog.h"
#include "ui/ParameterDialog.h"
#include "ui/LayerPaletteWidget.h"
#include "ui/LayoutViewWidget.h"
#include "ui/PropertyEditorWidget.h"
#include "ui/SchematicViewWidget.h"
#include "ui/SimSetupDialog.h"
#include "ui/WaveformViewWidget.h"

#include <QAction>
#include <fstream>
#include <functional>
#include <QActionGroup>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
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
#include <QTimer>
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
  addMenuAction(fileMenu, "&Quit", QKeySequence::Quit, qApp, SLOT(quit()));

  // Edit
  auto* editMenu = menuBar()->addMenu("&Edit");
  editMenu->addAction("Undo", this, &MainWindow::onUndo)->setShortcut(QKeySequence::Undo);
  editMenu->addAction("Redo", this, &MainWindow::onRedo)->setShortcut(QKeySequence::Redo);
  editMenu->addSeparator();
  editMenu->addAction("&Copy Shapes", this, &MainWindow::onCopyShapes)->setShortcut(QKeySequence::Copy);
  editMenu->addAction("&Paste Shapes", this, &MainWindow::onPasteShapes)->setShortcut(QKeySequence::Paste);
  editMenu->addSeparator();
  editMenu->addAction("Step and &Repeat…", this, &MainWindow::onStepRepeat);
  editMenu->addSeparator();
  editMenu->addAction("Instance &Parameters…", this, &MainWindow::onEditParameters);

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
    auto* a = toolMenu->addAction("&Label",         this, &MainWindow::onToolLabel);
    a->setShortcut(Qt::Key_L);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("Sti&mulus",      this, &MainWindow::onToolStimulus);
    a->setShortcut(Qt::Key_M);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Probe",         this, &MainWindow::onToolProbe);
    a->setShortcut(Qt::Key_B);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("S&ymbol Pin",    this, &MainWindow::onToolSymbolPin);
    a->setShortcut(Qt::Key_Y);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("Bus R&ip",       this, &MainWindow::onToolBusRip);
    a->setShortcut(Qt::Key_U);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("Place &Instance",this, &MainWindow::onToolInstance);
    a->setShortcut(Qt::Key_I);
    a->setCheckable(true);
  }
  toolMenu->addAction("Edit S&ymbol…", this, &MainWindow::onEditSymbol);
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
  {
    auto* a = toolMenu->addAction("P&ath",          this, &MainWindow::onToolPath);
    a->setShortcut(Qt::Key_A);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Via Array",     this, &MainWindow::onToolViaArray);
    a->setShortcut(Qt::Key_V);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Via (single)",  this, &MainWindow::onToolVia);
    a->setShortcut(Qt::Key_J);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Guard Ring",    this, &MainWindow::onToolGuardRing);
    a->setShortcut(Qt::Key_G);
    a->setCheckable(true);
  }
  {
    auto* a = toolMenu->addAction("&Ruler",         this, &MainWindow::onToolRuler);
    a->setShortcut(Qt::Key_D);
    a->setCheckable(true);
  }

  // Simulation
  auto* simMenu = menuBar()->addMenu("Si&mulation");
  {
    auto* a = simMenu->addAction("&Setup && Run…", this, &MainWindow::onSimSetup);
    a->setShortcut(Qt::CTRL | Qt::Key_R);
  }
  simMenu->addAction("View &Waveforms", this, [this] { tabs_->setCurrentIndex(2); });
  simMenu->addAction("FFT of First Trace", this, &MainWindow::onComputeFft);
  simMenu->addAction("Eye Diagram…", this, &MainWindow::onComputeEye);
  simMenu->addAction("Expression Editor…", this, &MainWindow::onExpressionEditor);
  simMenu->addAction("&Plot from Schematic", this, &MainWindow::onDirectPlot);

  // PCells
  auto* pcellMenu = menuBar()->addMenu("PCe&lls");
  pcellMenu->addAction("Generate &NMOS…", this, [this] {
    auto& lib = app_.projects().workingLibrary();
    // Create a new cell
    auto& cell = lib.createCell("NMOS_PCell");
    auto& lv   = cell.createView(db::DbViewType::Layout);
    bool ok = false;
    const QString wStr = QInputDialog::getText(this, "NMOS PCell",
        "Total width W (nm):", QLineEdit::Normal, "2000", &ok);
    if (!ok) return;
    const QString lStr = QInputDialog::getText(this, "NMOS PCell",
        "Gate length L (nm):", QLineEdit::Normal, "180", &ok);
    if (!ok) return;
    const QString fStr = QInputDialog::getText(this, "NMOS PCell",
        "Fingers:", QLineEdit::Normal, "1", &ok);
    if (!ok) return;
    pdk::ParamMap params;
    params["W"] = wStr.toStdString();
    params["L"] = lStr.toStdString();
    params["fingers"] = fStr.toStdString();
    if (app_.pcells().invoke("NMOS", lv, lib, app_.tech(), params)) {
      refreshLibraryBrowser();
      log("Generated NMOS PCell with W=" + wStr + " L=" + lStr + " fingers=" + fStr);
      statusBar()->showMessage("NMOS PCell generated", 4000);
    }
  });

  // Verification
  auto* verMenu = menuBar()->addMenu("&Verification");
  {
    auto* a = verMenu->addAction("&DRC / LVS Results…", this, &MainWindow::onDrcLvs);
    a->setShortcut(Qt::CTRL | Qt::Key_D);
  }

  // Import / Export
  auto* impMenu = menuBar()->addMenu("&Import/Export");
  impMenu->addAction("Import &SPICE Netlist…", this, &MainWindow::onImportSpice);
  impMenu->addAction("Import &GDS II…",        this, &MainWindow::onImportGds);
  impMenu->addAction("Import &DEF…",           this, &MainWindow::onImportDef);
  impMenu->addAction("Export &GDS II…",        this, &MainWindow::onExportGds);
  impMenu->addAction("Export &DEF…",           this, &MainWindow::onExportDef);
  impMenu->addAction("Export &Verilog…",       this, &MainWindow::onExportVerilog);
  impMenu->addAction("Export &LEF…",           this, &MainWindow::onExportLef);

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
  auto* idrcAct = toolbar->addAction("◉ iDRC"); idrcAct->setToolTip("Run Interactive DRC");
  connect(idrcAct, &QAction::triggered, this, &MainWindow::onRunIdrc);
  auto* layOpAct = toolbar->addAction("⊕ LayOp"); layOpAct->setToolTip("Layer Boolean Operation");
  connect(layOpAct, &QAction::triggered, this, &MainWindow::onLayerOperation);
  auto* chkAct  = toolbar->addAction("🔍 Sch"); chkAct->setToolTip("Check Schematic");
  connect(chkAct, &QAction::triggered, this, &MainWindow::onCheckSchematic);
  auto* snapAct = toolbar->addAction("⬕ Snap"); snapAct->setToolTip("Toggle Snap Mode (grid/object)");
  connect(snapAct, &QAction::triggered, this, &MainWindow::onSnapModeChanged);
  auto* conAct  = toolbar->addAction("⊕ Con"); conAct->setToolTip("Add Spacing Constraint");
  connect(conAct, &QAction::triggered, this, &MainWindow::onAddConstraint);
  auto* xlAct   = toolbar->addAction("⚡ XL"); xlAct->setToolTip("Generate Layout from Schematic");
  connect(xlAct, &QAction::triggered, this, &MainWindow::onGenerateFromSchematic);
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
  makeBtn("L", "Label (L)",          &MainWindow::onToolLabel);
  makeBtn("M", "Stimulus (M)",       &MainWindow::onToolStimulus);
  makeBtn("B", "Probe (B)",          &MainWindow::onToolProbe);
  makeBtn("Y", "Sym Pin (Y)",        &MainWindow::onToolSymbolPin);
  makeBtn("U", "Bus Rip (U)",        &MainWindow::onToolBusRip);
  toolbar->addSeparator();
  auto* busToggle = toolbar->addAction("≡"); busToggle->setToolTip("Toggle Bus Wire Mode");
  busToggle->setCheckable(true);
  connect(busToggle, &QAction::triggered, this, &MainWindow::onToggleBusMode);
  makeBtn("I", "Place Instance (I)", &MainWindow::onToolInstance);
  toolbar->addSeparator();
  makeBtn("R", "Rectangle (R)",      &MainWindow::onToolRect);
  makeBtn("P", "Polygon (P)",        &MainWindow::onToolPolygon);
  makeBtn("A", "Path (A)",           &MainWindow::onToolPath);
  makeBtn("V", "Via Array (V)",      &MainWindow::onToolViaArray);
  makeBtn("J", "Via (J)",            &MainWindow::onToolVia);
  makeBtn("G", "Guard Ring (G)",     &MainWindow::onToolGuardRing);
  makeBtn("D", "Ruler (D)",          &MainWindow::onToolRuler);

  // Navigation
  toolbar->addSeparator();
  auto* navUp = toolbar->addAction("▲"); navUp->setToolTip("Pop Up (go to parent cell)");
  connect(navUp, &QAction::triggered, this, &MainWindow::onNavigatePop);
  auto* xp = toolbar->addAction("⇋"); xp->setToolTip("Cross-Probe: highlight matching instances");
  connect(xp, &QAction::triggered, this, &MainWindow::onCrossProbe);

  // Alignment buttons (non-checkable, operate on current selection)
  toolbar->addSeparator();
  auto* al = toolbar->addAction("◀▶"); al->setToolTip("Align Left");
  connect(al, &QAction::triggered, this, &MainWindow::onAlignLeft);
  al = toolbar->addAction("▶◀"); al->setToolTip("Align Right");
  connect(al, &QAction::triggered, this, &MainWindow::onAlignRight);
  al = toolbar->addAction("▲▼"); al->setToolTip("Align Top");
  connect(al, &QAction::triggered, this, &MainWindow::onAlignTop);
  al = toolbar->addAction("▼▲"); al->setToolTip("Align Bottom");
  connect(al, &QAction::triggered, this, &MainWindow::onAlignBottom);
  al = toolbar->addAction("⇔"); al->setToolTip("Align Center H");
  connect(al, &QAction::triggered, this, &MainWindow::onAlignCenterH);
  al = toolbar->addAction("⇕"); al->setToolTip("Align Center V");
  connect(al, &QAction::triggered, this, &MainWindow::onAlignCenterV);
  al = toolbar->addAction("⊞"); al->setToolTip("Distribute H");
  connect(al, &QAction::triggered, this, &MainWindow::onDistributeH);
  al = toolbar->addAction("⊟"); al->setToolTip("Distribute V");
  connect(al, &QAction::triggered, this, &MainWindow::onDistributeV);
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
    pushUndoState();
    schCtrl_->setActiveTool(std::make_unique<schematic::SchToolWire>());
    statusBar()->showMessage("Wire tool active — click to start, click to add segment, Esc to finish", 4000);
  }
}

void MainWindow::onToolLabel() {
  if (!schCtrl_) return;
  tabs_->setCurrentIndex(0);
  pushUndoState();
  auto tool = std::make_unique<schematic::SchToolLabel>();
  tool->requestLabelName = [this](geom::GeomPoint) -> std::string {
    bool ok = false;
    const QString name = QInputDialog::getText(this, "Net Label",
        "Enter net name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return {};
    return name.toStdString();
  };
  schCtrl_->setActiveTool(std::move(tool));
  statusBar()->showMessage("Label tool — click on a wire to name the net", 4000);
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

void MainWindow::onToolPath() {
  if (layCtrl_) {
    tabs_->setCurrentIndex(1);
    auto tool = std::make_unique<layout::LayToolPath>();
    statusBar()->showMessage("Path tool — click to add vertices, Enter to commit, Esc to cancel", 4000);
    layCtrl_->setActiveTool(std::move(tool));
  }
}

void MainWindow::onToolViaArray() {
  if (!layCtrl_) return;
  tabs_->setCurrentIndex(1);
  auto tool = std::make_unique<layout::LayToolViaArray>();
  tool->params = {3, 3, 200, 400, 400};
  tool->requestParams = [this]() -> std::optional<layout::ViaArrayParams> {
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Via Array");
    auto* lay = new QFormLayout(dlg);
    auto* cols = new QLineEdit("3", dlg);
    auto* rows = new QLineEdit("3", dlg);
    auto* size = new QLineEdit("200", dlg);
    auto* spX  = new QLineEdit("400", dlg);
    auto* spY  = new QLineEdit("400", dlg);
    lay->addRow("Columns:", cols);
    lay->addRow("Rows:", rows);
    lay->addRow("Via size (nm):", size);
    lay->addRow("Spacing X (nm):", spX);
    lay->addRow("Spacing Y (nm):", spY);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
    lay->addRow(btns);
    connect(btns, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    if (dlg->exec() != QDialog::Accepted) { dlg->deleteLater(); return std::nullopt; }
    layout::ViaArrayParams p;
    p.columns = cols->text().toInt();
    p.rows = rows->text().toInt();
    p.viaSize = cols->text().toInt();
    p.spacingX = spX->text().toLongLong();
    p.spacingY = spY->text().toLongLong();
    p.viaSize = size->text().toLongLong();
    dlg->deleteLater();
    return p;
  };
  statusBar()->showMessage("Via Array — drag a rectangle to define the array area", 4000);
  layCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::onToolVia() {
  if (!layCtrl_) return;
  tabs_->setCurrentIndex(1);
  auto tool = std::make_unique<layout::LayToolVia>();
  tool->requestParams = [this]() -> std::optional<layout::ViaParams> {
    bool ok = false;
    int w = QInputDialog::getInt(this, "Via", "Contact width (nm):", 200, 10, 10000, 1, &ok);
    if (!ok) return std::nullopt;
    int h = QInputDialog::getInt(this, "Via", "Contact height (nm):", 200, 10, 10000, 1, &ok);
    if (!ok) return std::nullopt;
    int e1 = QInputDialog::getInt(this, "Via", "Enclosure layer 1 (nm):", 100, 0, 10000, 1, &ok);
    if (!ok) return std::nullopt;
    int e2 = QInputDialog::getInt(this, "Via", "Enclosure layer 2 (nm):", 100, 0, 10000, 1, &ok);
    if (!ok) return std::nullopt;
    return layout::ViaParams{static_cast<geom::DbUnit>(w), static_cast<geom::DbUnit>(h),
                             static_cast<geom::DbUnit>(e1), static_cast<geom::DbUnit>(e2)};
  };
  statusBar()->showMessage("Via tool — click to place a via", 4000);
  layCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::onToolGuardRing() {
  if (!layCtrl_) return;
  tabs_->setCurrentIndex(1);
  auto tool = std::make_unique<layout::LayToolGuardRing>();
  tool->requestParams = [this]() -> std::optional<layout::GuardRingParams> {
    bool ok = false;
    const QString w = QInputDialog::getText(this, "Guard Ring",
        "Ring width (nm):", QLineEdit::Normal, "400", &ok);
    if (!ok) return std::nullopt;
    const QString s = QInputDialog::getText(this, "Guard Ring",
        "Spacing from inner area (nm):", QLineEdit::Normal, "200", &ok);
    if (!ok) return std::nullopt;
    return layout::GuardRingParams{w.toLongLong(), s.toLongLong()};
  };
  statusBar()->showMessage("Guard Ring — drag a rectangle around the area to protect", 4000);
  layCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::onToolStimulus() {
  if (!schCtrl_) return;
  tabs_->setCurrentIndex(0);
  auto tool = std::make_unique<schematic::SchToolStimulus>();
  tool->requestParams = [this]() -> std::optional<schematic::StimulusParams> {
    QStringList types;
    types << "VDC" << "IDC" << "VPULSE" << "VSIN";
    bool ok = false;
    const QString chosen = QInputDialog::getItem(this, "Stimulus Marker",
        "Type:", types, 0, false, &ok);
    if (!ok) return std::nullopt;
    schematic::StimulusParams p;
    if (chosen == "VDC") {
      p.type = "vdc";
      const QString dc = QInputDialog::getText(this, "VDC", "DC Voltage:",
          QLineEdit::Normal, "1.8", &ok);
      if (!ok) return std::nullopt;
      p.values["dc"] = dc.toStdString();
    } else if (chosen == "IDC") {
      p.type = "idc";
      const QString dc = QInputDialog::getText(this, "IDC", "DC Current:",
          QLineEdit::Normal, "1e-3", &ok);
      if (!ok) return std::nullopt;
      p.values["dc"] = dc.toStdString();
    } else if (chosen == "VPULSE") {
      p.type = "vpulse";
      p.values["v1"] = "0"; p.values["v2"] = "1.8";
      p.values["td"] = "0"; p.values["tr"] = "1n";
      p.values["tf"] = "1n"; p.values["pw"] = "10n"; p.values["per"] = "20n";
    } else if (chosen == "VSIN") {
      p.type = "vsin";
      p.values["voff"] = "0"; p.values["vamp"] = "1";
      p.values["freq"] = "1e6";
    }
    return p;
  };
  statusBar()->showMessage("Stimulus tool — click on a wire to place a source marker", 4000);
  schCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::onToolProbe() {
  if (!schCtrl_) return;
  tabs_->setCurrentIndex(0);
  auto tool = std::make_unique<schematic::SchToolProbe>();
  tool->requestType = [this]() -> std::optional<std::string> {
    QStringList types; types << "Voltage Probe" << "Current Probe";
    bool ok = false;
    QString chosen = QInputDialog::getItem(this, "Probe", "Type:", types, 0, false, &ok);
    if (!ok) return std::nullopt;
    return (chosen == "Current Probe") ? std::string("iprobe") : std::string("vprobe");
  };
  statusBar()->showMessage("Probe tool — click on a wire to place a measurement probe", 4000);
  schCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::onToolBusRip() {
  if (!schCtrl_) return;
  tabs_->setCurrentIndex(0);
  auto tool = std::make_unique<schematic::SchToolBusRip>();
  tool->requestSignalName = [this]() -> std::optional<std::string> {
    bool ok = false;
    const QString name = QInputDialog::getText(this, "Bus Rip",
        "Signal name (e.g., data[3]):", QLineEdit::Normal, "sig", &ok);
    if (!ok || name.isEmpty()) return std::nullopt;
    return name.toStdString();
  };
  statusBar()->showMessage("Bus Rip — click on a bus wire to rip out a signal", 4000);
  schCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::onToggleBusMode() {
  // Toggle bus mode on the active wire tool
  if (auto* wt = dynamic_cast<schematic::SchToolWire*>(schCtrl_->activeTool())) {
    wt->setBusMode(!wt->busMode());
    statusBar()->showMessage(wt->busMode() ? "Bus wire mode ON" : "Bus wire mode OFF", 3000);
  } else {
    statusBar()->showMessage("Switch to Wire tool first", 3000);
  }
}

void MainWindow::onEditSymbol() {
  // Open the symbol view of the current cell
  auto& lib = app_.projects().workingLibrary();
  // Find first cell with symbol view
  for (const auto cid : lib.cellIds()) {
    auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    auto* sv = cell->findView(db::DbViewType::Symbol);
    if (!sv) { sv = &cell->createView(db::DbViewType::Symbol); }
    schDoc_ = std::make_unique<schematic::SchDocument>(*sv);
    schCtrl_ = std::make_unique<schematic::SchEditorController>(*schDoc_);
    schView_->setDocument(schDoc_.get(), &lib);
    schView_->setController(schCtrl_.get());
    tabs_->setCurrentIndex(0);
    statusBar()->showMessage(QString("Editing symbol: %1").arg(
        QString::fromStdString(cell->name())), 4000);
    return;
  }
  statusBar()->showMessage("No cells to edit symbol for", 3000);
}

void MainWindow::onToolSymbolPin() {
  if (!schCtrl_) return;
  tabs_->setCurrentIndex(0);
  auto tool = std::make_unique<schematic::SchToolSymbolPin>();
  tool->requestPin = [this]() -> std::optional<schematic::PinDefinition> {
    bool ok = false;
    const QString name = QInputDialog::getText(this, "Symbol Pin",
        "Pin name:", QLineEdit::Normal, "A", &ok);
    if (!ok || name.isEmpty()) return std::nullopt;
    QStringList dirs; dirs << "input" << "output" << "inout";
    const QString dir = QInputDialog::getItem(this, "Symbol Pin",
        "Direction:", dirs, 0, false, &ok);
    if (!ok) return std::nullopt;
    return schematic::PinDefinition{name.toStdString(), dir.toStdString()};
  };
  statusBar()->showMessage("Symbol Pin — click to place a pin on the symbol", 4000);
  schCtrl_->setActiveTool(std::move(tool));
}

void MainWindow::pushUndoState() {
  if (!schCtrl_) return;
  undoStack_.push_back("state");
  redoStack_.clear();
}

void MainWindow::onEditParameters() {
  if (!schCtrl_) return;
  const auto* sel = dynamic_cast<const schematic::SchToolSelect*>(schCtrl_->activeTool());
  if (!sel || sel->selectedInstances().empty()) {
    statusBar()->showMessage("Select an instance first to edit parameters", 4000);
    return;
  }
  auto& view = schCtrl_->document().view();
  auto* inst = view.findInstance(*sel->selectedInstances().begin());
  if (!inst) return;
  ParameterDialog dlg(*inst, this);
  if (dlg.exec() == QDialog::Accepted) {
    pushUndoState();
    schView_->update();
    statusBar()->showMessage("Parameters updated", 3000);
  }
}

void MainWindow::onCheckSchematic() {
  if (!schDoc_) { statusBar()->showMessage("No schematic loaded", 3000); return; }
  auto& view = schDoc_->view();
  QStringList warnings;
  for (const auto nid : view.netIds()) {
    const auto* net = view.findNet(nid);
    if (!net) continue;
    int connections = 0;
    for (const auto pid : view.pinIds()) {
      const auto* pin = view.findPin(pid);
      if (pin && pin->netId() == nid) ++connections;
    }
    if (connections <= 1 && net->name().find("net_") != 0)
      warnings << QString("Net '%1' has only %2 connection(s)").arg(
          QString::fromStdString(net->name())).arg(connections);
  }
  if (warnings.isEmpty())
    QMessageBox::information(this, "Check Schematic", "No issues found.");
  else
    QMessageBox::warning(this, "Check Schematic",
        QString("Found %1 issue(s):\n").arg(warnings.size()) + warnings.join("\n"));
}

// ─── Layout Undo / Redo ──────────────────────────────────────────────────────

void MainWindow::pushLayUndoState() {
  if (!layCtrl_) return;
  layUndoStack_.push_back("lay_state");
  layRedoStack_.clear();
}

void MainWindow::onUndo() {
  if (!undoStack_.empty()) {
    redoStack_.push_back(undoStack_.back());
    undoStack_.pop_back();
    if (!undoStack_.empty()) schView_->update();
    statusBar()->showMessage("Undo", 2000);
  } else if (!layUndoStack_.empty()) {
    layRedoStack_.push_back(layUndoStack_.back());
    layUndoStack_.pop_back();
    if (!layUndoStack_.empty()) layView_->update();
    statusBar()->showMessage("Undo", 2000);
  }
}

void MainWindow::onRedo() {
  if (!redoStack_.empty()) {
    undoStack_.push_back(redoStack_.back());
    redoStack_.pop_back();
    schView_->update();
    statusBar()->showMessage("Redo", 2000);
  } else if (!layRedoStack_.empty()) {
    layUndoStack_.push_back(layRedoStack_.back());
    layRedoStack_.pop_back();
    layView_->update();
    statusBar()->showMessage("Redo", 2000);
  }
}

// ─── Copy / Paste ────────────────────────────────────────────────────────────

void MainWindow::onCopyShapes() {
  if (!layCtrl_) return;
  const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
  if (!sel || sel->selectedShapes().empty()) {
    statusBar()->showMessage("Select shapes in layout first", 3000);
    return;
  }
  shapeClipboard_.assign(sel->selectedShapes().begin(), sel->selectedShapes().end());
  statusBar()->showMessage(QString("Copied %1 shape(s)").arg(shapeClipboard_.size()), 3000);
}

void MainWindow::onPasteShapes() {
  if (!layCtrl_ || shapeClipboard_.empty()) return;
  pushLayUndoState();
  auto& view = layCtrl_->document().view();
  auto& lib = app_.projects().workingLibrary();
  const geom::DbUnit offset = 5000; // 5µm paste offset

  for (const auto sid : shapeClipboard_) {
    const auto* orig = view.findShape(sid);
    if (!orig) continue;
    switch (orig->kind()) {
      case db::DbShapeKind::Rect: {
        auto b = static_cast<const db::DbRect*>(orig)->box();
        b.translate(offset, offset);
        (void)view.createRect(orig->layerId(), b);
        break;
      }
      case db::DbShapeKind::Polygon: {
        auto p = static_cast<const db::DbPolygon*>(orig)->polygon();
        p.translate(offset, offset);
        (void)view.createPolygon(orig->layerId(), p);
        break;
      }
      case db::DbShapeKind::Path: {
        auto p = static_cast<const db::DbPath*>(orig)->path();
        p.translate(offset, offset);
        (void)view.createPath(orig->layerId(), p);
        break;
      }
      case db::DbShapeKind::Text: {
        const auto* t = static_cast<const db::DbText*>(orig);
        (void)view.createText(orig->layerId(),
            {t->origin().x + offset, t->origin().y + offset}, t->text());
        break;
      }
    }
  }
  layView_->update();
  statusBar()->showMessage(QString("Pasted %1 shape(s)").arg(shapeClipboard_.size()), 3000);
}

// ─── Step and Repeat ─────────────────────────────────────────────────────────

void MainWindow::onStepRepeat() {
  if (!layCtrl_) return;
  const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
  if (!sel || sel->selectedShapes().empty()) {
    statusBar()->showMessage("Select shapes first", 3000);
    return;
  }
  bool ok = false;
  int cols = QInputDialog::getInt(this, "Step and Repeat", "Columns:", 2, 1, 100, 1, &ok);
  if (!ok) return;
  int rows = QInputDialog::getInt(this, "Step and Repeat", "Rows:", 2, 1, 100, 1, &ok);
  if (!ok) return;
  geom::DbUnit pitchX = QInputDialog::getInt(this, "Step and Repeat", "Pitch X (nm):", 10000, 1, 1000000, 1, &ok);
  if (!ok) return;
  geom::DbUnit pitchY = QInputDialog::getInt(this, "Step and Repeat", "Pitch Y (nm):", 10000, 1, 1000000, 1, &ok);
  if (!ok) return;

  pushLayUndoState();
  auto& view = layCtrl_->document().view();

  // For each selected shape, repeat it in a grid
  for (const auto sid : sel->selectedShapes()) {
    const auto* orig = view.findShape(sid);
    if (!orig) continue;
    for (int c = 0; c < cols; ++c) {
      for (int r = 0; r < rows; ++r) {
        if (c == 0 && r == 0) continue; // skip original
        const auto dx = c * pitchX;
        const auto dy = r * pitchY;
        switch (orig->kind()) {
          case db::DbShapeKind::Rect: {
            auto b = static_cast<const db::DbRect*>(orig)->box();
            b.translate(dx, dy);
            (void)view.createRect(orig->layerId(), b);
            break;
          }
          case db::DbShapeKind::Polygon: {
            auto p = static_cast<const db::DbPolygon*>(orig)->polygon();
            p.translate(dx, dy);
            (void)view.createPolygon(orig->layerId(), p);
            break;
          }
          case db::DbShapeKind::Path: {
            auto p = static_cast<const db::DbPath*>(orig)->path();
            p.translate(dx, dy);
            (void)view.createPath(orig->layerId(), p);
            break;
          }
          default: break;
        }
      }
    }
  }
  layView_->update();
  statusBar()->showMessage(QString("Repeated %1 x %2").arg(cols).arg(rows), 3000);
}

void MainWindow::onToolRuler() {
  if (!layCtrl_) return;
  tabs_->setCurrentIndex(1);
  layCtrl_->setActiveTool(std::make_unique<layout::LayToolRuler>());
  statusBar()->showMessage("Ruler — click two points to measure distance", 4000);
}

// ─── Hierarchical navigation ─────────────────────────────────────────────────

void MainWindow::onInstanceDoubleClicked() {
  if (!layCtrl_ && !schCtrl_) return;
  // Find which instance is under cursor by checking selection
  db::DbId targetCellId = db::kInvalidId;
  db::DbViewType currentType;
  db::DbId currentCellId = db::kInvalidId;

  const int tab = tabs_->currentIndex();
  if (tab == 0 && schCtrl_) {
    currentType = db::DbViewType::Schematic;
    const auto* sel = dynamic_cast<const schematic::SchToolSelect*>(schCtrl_->activeTool());
    if (!sel || sel->selectedInstances().empty()) {
      statusBar()->showMessage("Select an instance first, then double-click to push in", 4000);
      return;
    }
    const auto& view = schCtrl_->document().view();
    currentCellId = view.cellId();
    const auto* inst = view.findInstance(*sel->selectedInstances().begin());
    if (!inst) return;
    targetCellId = inst->masterCellId();
  } else if (tab == 1 && layCtrl_) {
    currentType = db::DbViewType::Layout;
    const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
    if (!sel || sel->selectedShapes().empty()) {
      statusBar()->showMessage("Select an instance first, then double-click to push in", 4000);
      return;
    }
    const auto& view = layCtrl_->document().view();
    currentCellId = view.cellId();
    const auto* inst = view.findInstance(*sel->selectedShapes().begin());
    if (!inst) return;
    targetCellId = inst->masterCellId();
  } else {
    return;
  }

  if (targetCellId == db::kInvalidId) return;

  // Push current cell onto nav stack
  navStack_.push_back({currentCellId, currentType});

  // Navigate to target cell
  const auto& lib = app_.projects().workingLibrary();
  const auto* targetCell = lib.findCellById(targetCellId);
  if (!targetCell) {
    statusBar()->showMessage("Target cell not found", 4000);
    navStack_.pop_back();
    return;
  }

  // Try to open schematic view, then layout view
  const auto* targetSch = targetCell->findView(db::DbViewType::Schematic);
  const auto* targetLay = targetCell->findView(db::DbViewType::Layout);
  if (!targetSch && !targetLay) {
    statusBar()->showMessage("Target cell has no schematic or layout view", 4000);
    navStack_.pop_back();
    return;
  }

  if (targetSch && tab == 0) {
    navigateToCell(targetCell->id(), db::DbViewType::Schematic);
  } else if (targetLay && tab == 1) {
    navigateToCell(targetCell->id(), db::DbViewType::Layout);
  } else if (targetSch) {
    navigateToCell(targetCell->id(), db::DbViewType::Schematic);
  } else {
    navigateToCell(targetCell->id(), db::DbViewType::Layout);
  }

  statusBar()->showMessage(
      QString("Pushed into %1").arg(QString::fromStdString(targetCell->name())), 4000);
}

void MainWindow::onNavigatePop() {
  if (navStack_.empty()) {
    statusBar()->showMessage("Already at top level", 3000);
    return;
  }
  const auto entry = navStack_.back();
  navStack_.pop_back();
  navigateToCell(entry.cellId, entry.viewType);
  statusBar()->showMessage("Popped back", 4000);
}

void MainWindow::onCrossProbe() {
  const int tab = tabs_->currentIndex();
  if (tab == 0 && schCtrl_) {
    // Schematic → Layout cross-probe
    const auto* sel = dynamic_cast<const schematic::SchToolSelect*>(schCtrl_->activeTool());
    if (!sel || sel->selectedInstances().empty()) {
      statusBar()->showMessage("Select an instance in schematic first", 4000);
      return;
    }
    const auto& view = schCtrl_->document().view();
    const auto* inst = view.findInstance(*sel->selectedInstances().begin());
    if (!inst) return;
    crossProbeCellId_ = inst->masterCellId();
    schView_->setCrossProbeCellId(crossProbeCellId_);
    layView_->setCrossProbeCellId(crossProbeCellId_);
    const auto* master = app_.projects().workingLibrary().findCellById(crossProbeCellId_);
    statusBar()->showMessage(QString("Cross-probe: %1").arg(
        master ? QString::fromStdString(master->name()) : "unknown"), 4000);
  } else if (tab == 1 && layCtrl_) {
    // Layout → Schematic cross-probe
    const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
    if (!sel || sel->selectedShapes().empty()) {
      statusBar()->showMessage("Select an instance in layout first", 4000);
      return;
    }
    const auto& view = layCtrl_->document().view();
    // Find the parent instance from selected shape
    for (const auto sid : sel->selectedShapes()) {
      const auto* shape = view.findShape(sid);
      if (!shape) continue;
      // Shape is selected, highlight instances of same master cell
      // For now, cross-probe by instance: find instance containing this shape
      (void)shape;
    }
    // Simplified: just clear the cross-probe
    crossProbeCellId_ = db::kInvalidId;
    schView_->setCrossProbeCellId(db::kInvalidId);
    layView_->setCrossProbeCellId(db::kInvalidId);
    statusBar()->showMessage("Cross-probe cleared", 4000);
  }
  // Clear cross-probe on second click
  auto reset = [this]() {
    crossProbeCellId_ = db::kInvalidId;
    schView_->setCrossProbeCellId(db::kInvalidId);
    layView_->setCrossProbeCellId(db::kInvalidId);
  };
  QTimer::singleShot(5000, this, reset);
}

void MainWindow::navigateToCell(db::DbId cellId, db::DbViewType viewType) {
  auto& lib = app_.projects().workingLibrary();
  auto* cell = lib.findCellById(cellId);
  if (!cell) return;

  if (viewType == db::DbViewType::Schematic || viewType == db::DbViewType::Symbol) {
    auto* sv = cell->findView(db::DbViewType::Schematic);
    if (!sv) sv = cell->findView(db::DbViewType::Symbol);
    if (!sv) { sv = &cell->createView(db::DbViewType::Schematic); }
    schDoc_ = std::make_unique<schematic::SchDocument>(*sv);
    schCtrl_ = std::make_unique<schematic::SchEditorController>(*schDoc_);
    schView_->setDocument(schDoc_.get(), &lib);
    schView_->setController(schCtrl_.get());
    tabs_->setCurrentIndex(0);
  } else {
    auto* lv = cell->findView(db::DbViewType::Layout);
    if (!lv) { lv = &cell->createView(db::DbViewType::Layout); }
    layDoc_ = std::make_unique<layout::LayDocument>(*lv);
    layCtrl_ = std::make_unique<layout::LayEditorController>(*layDoc_);
    layView_->setDocument(layDoc_.get(), &lib);
    layView_->setController(layCtrl_.get());
    tabs_->setCurrentIndex(1);
  }
  layView_->fitView();
  schView_->fitView();
}

// ─── Layout alignment helpers ─────────────────────────────────────────────────
// Align all selected shapes to the chosen edge/center of the first selected shape.

static geom::GeomBox shapeBoundingBox(const aurora::db::DbShape& shape) {
  switch (shape.kind()) {
    case aurora::db::DbShapeKind::Rect:
      return static_cast<const aurora::db::DbRect&>(shape).box();
    case aurora::db::DbShapeKind::Polygon:
      return static_cast<const aurora::db::DbPolygon&>(shape).polygon().boundingBox();
    case aurora::db::DbShapeKind::Path: {
      const auto& pts = static_cast<const aurora::db::DbPath&>(shape).path().points();
      if (pts.empty()) return {};
      geom::GeomBox bb{pts[0].x, pts[0].y, pts[0].x, pts[0].y};
      for (const auto& pt : pts)
        bb = geom::GeomBox{std::min(bb.left(), pt.x), std::min(bb.bottom(), pt.y),
                           std::max(bb.right(), pt.x), std::max(bb.top(), pt.y)};
      return bb;
    }
    default: return {};
  }
}

static void translateShape(aurora::db::DbShape& shape, geom::DbUnit dx, geom::DbUnit dy) {
  switch (shape.kind()) {
    case aurora::db::DbShapeKind::Rect: {
      auto& r = static_cast<aurora::db::DbRect&>(shape);
      auto b = r.box();
      b.translate(dx, dy);
      r.setBox(b);
      break;
    }
    case aurora::db::DbShapeKind::Polygon:
      static_cast<aurora::db::DbPolygon&>(shape).polygon().translate(dx, dy);
      break;
    case aurora::db::DbShapeKind::Path:
      static_cast<aurora::db::DbPath&>(shape).path().translate(dx, dy);
      break;
    default: break;
  }
}

static void alignHorizontal(aurora::layout::LayEditorController& ctrl,
                             std::function<geom::DbUnit(const geom::GeomBox&)> getEdge) {
  auto& view = ctrl.document().view();
  const auto* selTool = dynamic_cast<const aurora::layout::LayToolSelect*>(ctrl.activeTool());
  if (!selTool || selTool->selectedShapes().empty()) return;
  const auto& ids = selTool->selectedShapes();
  auto* first = view.findShape(*ids.begin());
  if (!first) return;
  const geom::DbUnit target = getEdge(shapeBoundingBox(*first));
  for (const auto sid : ids) {
    auto* shape = view.findShape(sid);
    if (!shape) continue;
    const geom::DbUnit current = getEdge(shapeBoundingBox(*shape));
    const geom::DbUnit dx = target - current;
    if (dx != 0) translateShape(*shape, dx, 0);
  }
}

static void alignVertical(aurora::layout::LayEditorController& ctrl,
                           std::function<geom::DbUnit(const geom::GeomBox&)> getEdge) {
  auto& view = ctrl.document().view();
  const auto* selTool = dynamic_cast<const aurora::layout::LayToolSelect*>(ctrl.activeTool());
  if (!selTool || selTool->selectedShapes().empty()) return;
  const auto& ids = selTool->selectedShapes();
  auto* first = view.findShape(*ids.begin());
  if (!first) return;
  const geom::DbUnit target = getEdge(shapeBoundingBox(*first));
  for (const auto sid : ids) {
    auto* shape = view.findShape(sid);
    if (!shape) continue;
    const geom::DbUnit current = getEdge(shapeBoundingBox(*shape));
    const geom::DbUnit dy = target - current;
    if (dy != 0) translateShape(*shape, 0, dy);
  }
}

void MainWindow::onAlignLeft() {
  if (!layCtrl_) { statusBar()->showMessage("No layout active", 3000); return; }
  alignHorizontal(*layCtrl_, [](const geom::GeomBox& b) { return b.left(); });
  layView_->update();
}

void MainWindow::onAlignRight() {
  if (!layCtrl_) { statusBar()->showMessage("No layout active", 3000); return; }
  alignHorizontal(*layCtrl_, [](const geom::GeomBox& b) { return b.right(); });
  layView_->update();
}

void MainWindow::onAlignTop() {
  if (!layCtrl_) { statusBar()->showMessage("No layout active", 3000); return; }
  alignVertical(*layCtrl_, [](const geom::GeomBox& b) { return b.top(); });
  layView_->update();
}

void MainWindow::onAlignBottom() {
  if (!layCtrl_) { statusBar()->showMessage("No layout active", 3000); return; }
  alignVertical(*layCtrl_, [](const geom::GeomBox& b) { return b.bottom(); });
  layView_->update();
}

void MainWindow::onAlignCenterH() {
  if (!layCtrl_) { statusBar()->showMessage("No layout active", 3000); return; }
  alignHorizontal(*layCtrl_, [](const geom::GeomBox& b) { return (b.left() + b.right()) / 2; });
  layView_->update();
}

void MainWindow::onAlignCenterV() {
  if (!layCtrl_) { statusBar()->showMessage("No layout active", 3000); return; }
  alignVertical(*layCtrl_, [](const geom::GeomBox& b) { return (b.bottom() + b.top()) / 2; });
  layView_->update();
}

void MainWindow::onDistributeH() {
  if (!layCtrl_) return;
  auto& view = layCtrl_->document().view();
  const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
  if (!sel || sel->selectedShapes().size() < 3) {
    statusBar()->showMessage("Select 3+ shapes to distribute", 4000);
    return;
  }
  pushLayUndoState();
  // Collect X centers, sorted
  std::vector<std::pair<geom::DbUnit, db::DbId>> items;
  for (const auto sid : sel->selectedShapes()) {
    const auto* s = view.findShape(sid);
    if (!s) continue;
    auto bb = shapeBoundingBox(*s);
    items.emplace_back((bb.left() + bb.right()) / 2, sid);
  }
  std::sort(items.begin(), items.end());
  // First and last stay fixed, distribute the ones between
  const geom::DbUnit xFirst = items.front().first;
  const geom::DbUnit xLast = items.back().first;
  const double step = static_cast<double>(xLast - xFirst) / (items.size() - 1);
  for (std::size_t i = 1; i + 1 < items.size(); ++i) {
    auto* shape = view.findShape(items[i].second);
    if (!shape) continue;
    auto bb = shapeBoundingBox(*shape);
    const geom::DbUnit target = static_cast<geom::DbUnit>(xFirst + i * step);
    translateShape(*shape, target - items[i].first, 0);
  }
  layView_->update();
}

void MainWindow::onDistributeV() {
  if (!layCtrl_) return;
  auto& view = layCtrl_->document().view();
  const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
  if (!sel || sel->selectedShapes().size() < 3) {
    statusBar()->showMessage("Select 3+ shapes to distribute", 4000);
    return;
  }
  pushLayUndoState();
  std::vector<std::pair<geom::DbUnit, db::DbId>> items;
  for (const auto sid : sel->selectedShapes()) {
    const auto* s = view.findShape(sid);
    if (!s) continue;
    auto bb = shapeBoundingBox(*s);
    items.emplace_back((bb.bottom() + bb.top()) / 2, sid);
  }
  std::sort(items.begin(), items.end());
  const geom::DbUnit yFirst = items.front().first;
  const geom::DbUnit yLast = items.back().first;
  const double step = static_cast<double>(yLast - yFirst) / (items.size() - 1);
  for (std::size_t i = 1; i + 1 < items.size(); ++i) {
    auto* shape = view.findShape(items[i].second);
    if (!shape) continue;
    auto bb = shapeBoundingBox(*shape);
    const geom::DbUnit target = static_cast<geom::DbUnit>(yFirst + i * step);
    translateShape(*shape, 0, target - items[i].first);
  }
  layView_->update();
}

// ─── Stretch tool ────────────────────────────────────────────────────────────

void MainWindow::onToolStretch() {
  if (!layCtrl_) return;
  tabs_->setCurrentIndex(1);
  layCtrl_->setActiveTool(std::make_unique<layout::LayToolStretch>());
  statusBar()->showMessage("Stretch — click and drag an edge of a rectangle", 4000);
}

// ─── Constraints ─────────────────────────────────────────────────────────────

void MainWindow::onAddConstraint() {
  if (!layCtrl_) return;
  const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
  if (!sel || sel->selectedShapes().size() < 2) {
    statusBar()->showMessage("Select 2+ shapes to constrain", 3000);
    return;
  }
  auto& view = layCtrl_->document().view();
  auto& con = view.createConstraint("spacing");
  for (const auto sid : sel->selectedShapes())
    con.addObject(sid);
  statusBar()->showMessage("Spacing constraint added", 3000);
}

// ─── Generate From Schematic (Layout XL) ─────────────────────────────────────

void MainWindow::onGenerateFromSchematic() {
  auto& lib = app_.projects().workingLibrary();
  // Find first schematic view and create corresponding layout cells
  for (const auto cid : lib.cellIds()) {
    auto* schCell = lib.findCellById(cid);
    if (!schCell) continue;
    auto* sv = schCell->findView(db::DbViewType::Schematic);
    if (!sv) continue;

    // Create or find layout view for this cell
    auto* lv = schCell->findView(db::DbViewType::Layout);
    if (!lv) lv = &schCell->createView(db::DbViewType::Layout);

    // For each instance in schematic, ensure a layout cell exists
    for (const auto iid : sv->instanceIds()) {
      const auto* inst = sv->findInstance(iid);
      if (!inst) continue;
      auto* masterCell = lib.findCellById(inst->masterCellId());
      if (!masterCell) continue;
      // Ensure master has a layout view
      if (!masterCell->findView(db::DbViewType::Layout))
        (void)masterCell->createView(db::DbViewType::Layout);
    }
    navigateToCell(cid, db::DbViewType::Layout);
    statusBar()->showMessage("Layout generated from schematic", 4000);
    return;
  }
  statusBar()->showMessage("No schematic found to generate from", 3000);
}

// ─── Snap Mode ───────────────────────────────────────────────────────────────

void MainWindow::onSnapModeChanged() {
  if (!layCtrl_) return;
  static bool objSnap = false;
  objSnap = !objSnap;
  layCtrl_->setSnapMode(objSnap ? layout::LayEditorController::SnapToObject
                                : layout::LayEditorController::SnapToGrid);
  statusBar()->showMessage(objSnap ? "Object snap ON" : "Grid snap ON", 3000);
}

// ─── Interactive DRC ─────────────────────────────────────────────────────────

void MainWindow::onRunIdrc() {
  if (!layDoc_) { statusBar()->showMessage("No layout loaded", 3000); return; }
  drc_lvs::DrcEngine engine(app_.tech());
  auto violations = engine.run(layDoc_->view(), app_.projects().workingLibrary());
  if (violations.empty()) {
    statusBar()->showMessage("DRC: No violations found", 4000);
  } else {
    statusBar()->showMessage(QString("DRC: %1 violation(s)").arg(violations.size()), 6000);
    log(QString("DRC found %1 violations:").arg(violations.size()));
    for (const auto& v : violations) {
      auto typeStr = [](auto t) -> std::string {
        switch (t) {
          case drc_lvs::DrcViolationType::MinWidth: return "MinWidth";
          case drc_lvs::DrcViolationType::MinSpacing: return "MinSpacing";
          case drc_lvs::DrcViolationType::Enclosure: return "Enclosure";
          case drc_lvs::DrcViolationType::NonManhattan: return "NonManhattan";
          default: return "Unknown";
        }
      };
      log(QString("  %1 on %2: %3").arg(
          QString::fromStdString(typeStr(v.type)),
          QString::fromStdString(v.layerName),
          QString::fromStdString(v.message)));
    }
  }
}

// ─── Layer Operations ────────────────────────────────────────────────────────

void MainWindow::onLayerOperation() {
  if (!layCtrl_) return;
  const auto* sel = dynamic_cast<const layout::LayToolSelect*>(layCtrl_->activeTool());
  if (!sel || sel->selectedShapes().size() < 2) {
    statusBar()->showMessage("Select 2+ shapes for layer operation", 3000);
    return;
  }
  pushLayUndoState();
  auto& view = layCtrl_->document().view();
  // Simple AND/OR: create a new rect covering the intersection/union of selected rects
  auto first = view.findShape(*sel->selectedShapes().begin());
  if (!first || first->kind() != db::DbShapeKind::Rect) return;
  auto bb = static_cast<const db::DbRect*>(first)->box();
  for (const auto sid : sel->selectedShapes()) {
    auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& b = static_cast<const db::DbRect*>(s)->box();
    bb = geom::GeomBox{std::min(bb.left(), b.left()), std::min(bb.bottom(), b.bottom()),
                        std::max(bb.right(), b.right()), std::max(bb.top(), b.top())};
  }
  (void)view.createRect(first->layerId(), bb);
  layView_->update();
  statusBar()->showMessage("Layer union created", 3000);
}

// ─── Grid ────────────────────────────────────────────────────────────────────

void MainWindow::onToggleGridType() {
  if (!layCtrl_) return;
  static bool ortho = false;
  ortho = !ortho;
  if (layCtrl_) {
    // Toggle grid type (ortho mode affects tool behavior)
    statusBar()->showMessage(ortho ? "Orthogonal mode ON" : "Orthogonal mode OFF", 3000);
  }
}

// ─── FFT ─────────────────────────────────────────────────────────────────────

void MainWindow::onComputeFft() {
  waveView_->computeFFT();
  tabs_->setCurrentIndex(2);
  statusBar()->showMessage("FFT computed", 3000);
}

void MainWindow::onComputeEye() {
  bool ok = false;
  double period = QInputDialog::getDouble(this, "Eye Diagram",
      "Period (s):", 1e-9, 1e-15, 1, 6, &ok);
  if (!ok || period <= 0) return;
  waveView_->computeEyeDiagram(period);
  tabs_->setCurrentIndex(2);
  statusBar()->showMessage("Eye diagram computed", 3000);
}

void MainWindow::onExpressionEditor() {
  if (waveView_->traceCount() < 2) {
    statusBar()->showMessage("Need 2+ traces for expression math", 4000);
    return;
  }
  QStringList names;
  for (int i = 0; i < waveView_->traceCount(); ++i)
    names << QString::fromStdString(waveView_->traceName(i));
  ExpressionDialog dlg(names, this);
  if (dlg.exec() == QDialog::Accepted) {
    std::string expr = dlg.expression().toStdString();
    std::string resultName = dlg.resultName().toStdString();
    if (!expr.empty() && !resultName.empty()) {
      waveView_->addExpressionTrace(expr, resultName, QColor("#ff44cc"));
      tabs_->setCurrentIndex(2);
    }
  }
}

// ─── Direct Plot from Schematic ─────────────────────────────────────────────

void MainWindow::onDirectPlot() {
  if (!schCtrl_ || !simRunner_) return;
  // Find the selected net from the schematic
  const auto* sel = dynamic_cast<const schematic::SchToolSelect*>(schCtrl_->activeTool());
  if (!sel || sel->selectedInstances().empty()) {
    statusBar()->showMessage("Select an instance/net in schematic first", 4000);
    return;
  }
  // For simplicity, plot the first selected instance's master cell name
  const auto& view = schCtrl_->document().view();
  const auto* inst = view.findInstance(*sel->selectedInstances().begin());
  if (!inst) return;
  const auto* master = app_.projects().workingLibrary().findCellById(inst->masterCellId());
  if (!master) return;

  // Look for waveform data matching this name
  const std::string searchName = master->name();
  // Trigger a re-simulation with direct plot would be complex
  // For now: switch to waveform tab and log
  tabs_->setCurrentIndex(2);
  statusBar()->showMessage(QString("Plot: %1 (re-run sim with probes on this net)").arg(
      QString::fromStdString(searchName)), 4000);
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

    // DC operating point annotation (B10): store for schematic display
    dcAnnotation_ = result.dcOperatingPoint;
    if (schView_) schView_->setDcAnnotation(dcAnnotation_);
    if (schView_) schView_->update();
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

void MainWindow::onImportGds() {
  const auto path = QFileDialog::getOpenFileName(this, "Import GDS II",
      QDir::homePath(), "GDS II (*.gds);;All Files (*)");
  if (path.isEmpty()) return;
  layout::LayGdsReader reader;
  if (reader.read(app_.projects().workingLibrary(), path.toStdString())) {
    refreshLibraryBrowser();
    log("Imported GDS II: " + path);
    statusBar()->showMessage("GDS import successful", 4000);
    // Switch to layout view if possible
    tabs_->setCurrentIndex(1);
    if (layView_) layView_->fitView();
  } else {
    QMessageBox::warning(this, "Import Error", "Failed to read GDS II file.");
    log("GDS import failed: " + path);
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

void MainWindow::onImportDef() {
  const auto path = QFileDialog::getOpenFileName(this, "Import DEF",
      QDir::homePath(), "DEF (*.def);;All Files (*)");
  if (path.isEmpty()) return;
  layout::LayDefReader reader;
  if (reader.read(app_.projects().workingLibrary(), path.toStdString())) {
    refreshLibraryBrowser();
    log("Imported DEF: " + path);
    statusBar()->showMessage("DEF import successful", 4000);
  } else {
    QMessageBox::warning(this, "Import Error", "Failed to read DEF file.");
  }
}

void MainWindow::onExportDef() {
  const auto path = QFileDialog::getSaveFileName(this, "Export DEF",
      QDir::homePath(), "DEF (*.def);;All Files (*)");
  if (path.isEmpty()) return;
  layout::LayDefWriter writer;
  if (writer.write(app_.projects().workingLibrary(), path.toStdString())) {
    log("DEF exported: " + path);
    statusBar()->showMessage("DEF exported", 4000);
  } else {
    QMessageBox::warning(this, "Export Error", "Failed to write DEF file.");
  }
}

void MainWindow::onExportVerilog() {
  const auto path = QFileDialog::getSaveFileName(this, "Export Verilog",
      QDir::homePath(), "Verilog (*.v);;All Files (*)");
  if (path.isEmpty()) return;
  auto& lib = app_.projects().workingLibrary();
  // Find the first cell with a schematic view
  for (const auto cid : lib.cellIds()) {
    auto* cell = lib.findCellById(cid);
    if (!cell) continue;
    auto* view = cell->findView(db::DbViewType::Schematic);
    if (!view) continue;
    netlist::VerilogGenerator gen;
    const std::string vlog = gen.generateVerilog(lib, *cell, *view);
    std::ofstream ofs(path.toStdString());
    if (ofs) { ofs << vlog; log("Verilog exported: " + path); statusBar()->showMessage("Verilog exported", 4000); }
    else { QMessageBox::warning(this, "Export Error", "Failed to write Verilog file."); }
    return;
  }
  statusBar()->showMessage("No schematic view found to export", 4000);
}

void MainWindow::onExportLef() {
  const auto path = QFileDialog::getSaveFileName(this, "Export LEF",
      QDir::homePath(), "LEF (*.lef);;All Files (*)");
  if (path.isEmpty()) return;
  layout::LayLefWriter writer;
  if (writer.write(app_.projects().workingLibrary(), path.toStdString())) {
    log("LEF exported: " + path);
    statusBar()->showMessage("LEF exported", 4000);
  } else {
    QMessageBox::warning(this, "Export Error", "Failed to write LEF file.");
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
