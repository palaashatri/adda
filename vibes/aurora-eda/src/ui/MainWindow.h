#pragma once

#include "core/CoreApp.h"
#include "layout/LayDocument.h"
#include "layout/LayEditorController.h"
#include "schematic/SchDocument.h"
#include "sim/SimResult.h"

#include <QMainWindow>

#include <memory>

class QAction;
class QActionGroup;
class QLabel;
class QPlainTextEdit;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace aurora {
namespace sim { class SimRunner; }
namespace schematic { class SchEditorController; }
}  // namespace aurora

namespace aurora::ui {

class CellBrowserDialog;
class DrcResultsDialog;
class LayoutViewWidget;
class LayerPaletteWidget;
class PropertyEditorWidget;
class SchematicViewWidget;
class SimSetupDialog;
class WaveformViewWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(aurora::core::CoreApp& app, QWidget* parent = nullptr);
  ~MainWindow() override;

 private slots:
  void onNewProject();
  void onOpenProject();
  void onSaveProject();
  void onZoomIn();
  void onZoomOut();
  void onZoomFit();
  void onAbout();
  void onCoordinatesChanged(QPointF scenePt);
  void onCrossProbe();

  // Tools
  void onToolSelect();
  void onToolWire();
  void onToolLabel();
  void onToolInstance();
  void onToolRect();
  void onToolPolygon();
  void onToolPath();
  void onToolViaArray();
  void onToolGuardRing();
  void onToolRuler();
  void onToolStimulus();

  // Hierarchical navigation
  void onInstanceDoubleClicked();
  void onNavigatePop();

  // Alignment (layout)
  void onAlignLeft();
  void onAlignRight();
  void onAlignTop();
  void onAlignBottom();
  void onAlignCenterH();
  void onAlignCenterV();
  void onDistributeH();
  void onDistributeV();

  // Simulation
  void onSimSetup();
  void onSimFinished(const sim::SimResult& result);

  // Verification
  void onDrcLvs();

  // Import / Export
  void onImportSpice();
  void onExportGds();
  void onExportDef();
  void onImportDef();
  void onExportVerilog();
  void onExportLef();
  void onImportGds();

  // Library
  void onLibraryItemDoubleClicked(QTreeWidgetItem* item, int col);

 private:
  void setupMenuBar();
  void setupToolBar();
  void setupToolToolBar();
  void setupDocks();
  void createDemoData();
  void refreshLibraryBrowser();
  void log(const QString& message);
  void navigateToCell(db::DbId cellId, db::DbViewType viewType);

  aurora::core::CoreApp& app_;
  QTabWidget* tabs_{nullptr};
  QPlainTextEdit* logPane_{nullptr};
  QLabel* coordLabel_{nullptr};
  QTreeWidget* libraryTree_{nullptr};
  SchematicViewWidget*  schView_{nullptr};
  LayoutViewWidget*     layView_{nullptr};
  WaveformViewWidget*   waveView_{nullptr};
  LayerPaletteWidget*   layerPalette_{nullptr};
  PropertyEditorWidget* propWidget_{nullptr};

  std::unique_ptr<layout::LayDocument>            layDoc_;
  std::unique_ptr<layout::LayEditorController>    layCtrl_;
  std::unique_ptr<schematic::SchDocument>         schDoc_;
  std::unique_ptr<schematic::SchEditorController> schCtrl_;
  std::unique_ptr<sim::SimRunner>                 simRunner_;

  // Cross-probing
  db::DbId crossProbeCellId_{db::kInvalidId};

  SimSetupDialog*    simDialog_{nullptr};
  DrcResultsDialog*  drcDialog_{nullptr};

  QActionGroup* schToolGroup_{nullptr};
  QActionGroup* layToolGroup_{nullptr};

  // Hierarchical navigation stack: pairs of (cellId, viewType)
  struct NavEntry { db::DbId cellId; db::DbViewType viewType; };
  std::vector<NavEntry> navStack_;
};

}  // namespace aurora::ui
