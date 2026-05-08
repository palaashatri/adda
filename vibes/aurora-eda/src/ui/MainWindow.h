#pragma once

#include "core/CoreApp.h"
#include "layout/LayDocument.h"
#include "layout/LayEditorController.h"
#include "schematic/SchDocument.h"

#include <QMainWindow>

#include <memory>

class QLabel;
class QPlainTextEdit;
class QTabWidget;
class QTreeWidget;

namespace aurora::ui {

class LayoutViewWidget;
class SchematicViewWidget;
class LayerPaletteWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(aurora::core::CoreApp& app, QWidget* parent = nullptr);

 private slots:
  void onNewProject();
  void onOpenProject();
  void onSaveProject();
  void onZoomIn();
  void onZoomOut();
  void onZoomFit();
  void onAbout();
  void onCoordinatesChanged(QPointF scenePt);

 private:
  void setupMenuBar();
  void setupToolBar();
  void setupDocks();
  void createDemoData();
  void refreshLibraryBrowser();
  void log(const QString& message);

  aurora::core::CoreApp& app_;
  QTabWidget* tabs_{nullptr};
  QPlainTextEdit* logPane_{nullptr};
  QLabel* coordLabel_{nullptr};
  QTreeWidget* libraryTree_{nullptr};
  SchematicViewWidget* schView_{nullptr};
  LayoutViewWidget* layView_{nullptr};
  LayerPaletteWidget*  layerPalette_{nullptr};
  std::unique_ptr<layout::LayDocument>    layDoc_;
  std::unique_ptr<layout::LayEditorController> layCtrl_;
  std::unique_ptr<schematic::SchDocument> schDoc_;
};

}  // namespace aurora::ui
