#pragma once

#include "db/DbTypes.h"
#include "geom/GeomPoint.h"
#include "schematic/SchTool.h"

#include <QPointF>
#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
class QPainter;

namespace aurora {
namespace db {
class DbCellLib;
class DbView;
}
namespace schematic {
class SchDocument;
class SchEditorController;
}
}  // namespace aurora

namespace aurora::ui {

class SchematicViewWidget : public QWidget {
  Q_OBJECT

 public:
  explicit SchematicViewWidget(QWidget* parent = nullptr);

  void setDocument(const schematic::SchDocument* doc, const db::DbCellLib* lib,
                   double dbuPerMicron = 1000.0);
  void setController(schematic::SchEditorController* ctrl);
  void fitView();
  void setCrossProbeCellId(db::DbId cellId) { crossProbeCellId_ = cellId; update(); }
  void setDcAnnotation(const std::map<std::string, double>& ann) { dcAnnotation_ = ann; }
  [[nodiscard]] const std::map<std::string, double>& dcAnnotation() const { return dcAnnotation_; }

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
  [[nodiscard]] double  dbuToScene(long long dbu) const;
  [[nodiscard]] geom::GeomPoint toDbPoint(QPointF screenPt) const;
  [[nodiscard]] static schematic::SchKeyEvent mapKey(int qtKey);

  void paintGrid(QPainter& painter) const;
  void paintDocument(QPainter& painter) const;
  void paintSymbol(QPainter& painter, const db::DbView& view, long long dx, long long dy) const;
  void paintToolOverlay(QPainter& painter) const;

  const schematic::SchDocument*   doc_{nullptr};
  const db::DbCellLib*            lib_{nullptr};
  schematic::SchEditorController* ctrl_{nullptr};
  double dbuPerMicron_{1000.0};

  double  zoom_{50.0};
  QPointF pan_{40.0, 40.0};
  QPointF lastMousePos_;
  QPointF selectedScene_;
  bool    panning_{false};
  bool    hasSelection_{false};
  db::DbId crossProbeCellId_{db::kInvalidId};
  std::map<std::string, double> dcAnnotation_;
};

}  // namespace aurora::ui
