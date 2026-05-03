#pragma once

#include <QPointF>
#include <QWidget>

class QMouseEvent;

namespace aurora::ui {

class SchematicViewWidget : public QWidget {
 public:
  explicit SchematicViewWidget(QWidget* parent = nullptr);

 protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

 private:
  [[nodiscard]] QPointF sceneToScreen(QPointF point) const;
  [[nodiscard]] QPointF screenToScene(QPointF point) const;

  double zoom_{1.0};
  QPointF pan_{40.0, 40.0};
  QPointF lastMousePosition_{0.0, 0.0};
  QPointF selectedScenePoint_{0.0, 0.0};
  bool panning_{false};
  bool hasSelection_{false};
};

}  // namespace aurora::ui
