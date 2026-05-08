#pragma once

#include "db/DbTypes.h"

#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QWidget>

#include <set>

class QKeyEvent;
class QMouseEvent;
class QPainter;

namespace aurora {
namespace db {
class DbCellLib;
class DbView;
}
namespace layout {
class LayDocument;
class LayEditorController;
}
}  // namespace aurora

namespace aurora::ui {

class LayoutViewWidget : public QWidget {
  Q_OBJECT

 public:
  explicit LayoutViewWidget(QWidget* parent = nullptr);

  void setDocument(const layout::LayDocument* doc, const db::DbCellLib* lib,
                   double dbuPerMicron = 1000.0);
  void setController(layout::LayEditorController* ctrl);
  void fitView();

  void setLayerVisible(db::DbId layerId, bool visible);
  void zoomToBox(double sceneX, double sceneY, double sceneW, double sceneH);

 signals:
  void coordinatesChanged(QPointF scenePt);

 protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  [[nodiscard]] QPointF sceneToScreen(QPointF p) const;
  [[nodiscard]] QPointF screenToScene(QPointF p) const;
  [[nodiscard]] QRectF  sceneRectToScreen(const QRectF& r) const;
  [[nodiscard]] double  dbuToScene(long long dbu) const;

  void paintGrid(QPainter& painter) const;
  void paintDocument(QPainter& painter) const;
  void paintView(QPainter& painter, const db::DbView& view, long long dx, long long dy) const;
  void paintToolOverlay(QPainter& painter) const;

  const layout::LayDocument*    doc_{nullptr};
  const db::DbCellLib*          lib_{nullptr};
  layout::LayEditorController*  ctrl_{nullptr};
  double dbuPerMicron_{1000.0};

  std::set<db::DbId> hiddenLayers_;

  double  zoom_{50.0};
  QPointF pan_{64.0, 64.0};
  QPointF lastMousePos_;
  QPointF selectedScene_;
  bool    panning_{false};
  bool    hasSelection_{false};
};

}  // namespace aurora::ui
