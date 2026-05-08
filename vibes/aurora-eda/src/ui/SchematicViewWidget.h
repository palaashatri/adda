#pragma once

#include <QPointF>
#include <QWidget>

class QMouseEvent;

namespace aurora {
namespace db {
class DbCellLib;
class DbView;
}
namespace schematic { class SchDocument; }
}

namespace aurora::ui {

class SchematicViewWidget : public QWidget {
  Q_OBJECT

 public:
  explicit SchematicViewWidget(QWidget* parent = nullptr);

  void setDocument(const schematic::SchDocument* doc, const db::DbCellLib* lib, double dbuPerMicron = 1000.0);
  void fitView();

 signals:
  void coordinatesChanged(QPointF scenePt);

 protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

 private:
  [[nodiscard]] QPointF sceneToScreen(QPointF p) const;
  [[nodiscard]] QPointF screenToScene(QPointF p) const;
  [[nodiscard]] double  dbuToScene(long long dbu) const;

  void paintGrid(QPainter& painter) const;
  void paintDocument(QPainter& painter) const;
  void paintSymbol(QPainter& painter, const db::DbView& view, long long dx, long long dy) const;

  const schematic::SchDocument* doc_{nullptr};
  const db::DbCellLib*          lib_{nullptr};
  double dbuPerMicron_{1000.0};

  double  zoom_{50.0};
  QPointF pan_{40.0, 40.0};
  QPointF lastMousePos_;
  QPointF selectedScene_;
  bool    panning_{false};
  bool    hasSelection_{false};
};

}  // namespace aurora::ui
