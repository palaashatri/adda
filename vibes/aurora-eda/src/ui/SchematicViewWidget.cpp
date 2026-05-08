#include "ui/SchematicViewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace aurora::ui {

SchematicViewWidget::SchematicViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 300);
  setMouseTracking(true);
}

void SchematicViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    const auto delta = event->position() - lastMousePosition_;
    pan_ += delta;
    lastMousePosition_ = event->position();
    update();
  }
}

void SchematicViewWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = true;
    lastMousePosition_ = event->position();
    setCursor(Qt::ClosedHandCursor);
    return;
  }

  if (event->button() == Qt::LeftButton) {
    selectedScenePoint_ = screenToScene(event->position());
    hasSelection_ = true;
    update();
  }
}

void SchematicViewWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = false;
    unsetCursor();
  }
}

void SchematicViewWidget::paintEvent(QPaintEvent* event) {
  (void)event;

  QPainter painter(this);
  painter.fillRect(rect(), QColor("#fbfbf8"));
  painter.setRenderHint(QPainter::Antialiasing, false);

  const double step = std::max(4.0, 20.0 * zoom_);
  auto startX = std::fmod(pan_.x(), step);
  auto startY = std::fmod(pan_.y(), step);
  if (startX < 0.0) {
    startX += step;
  }
  if (startY < 0.0) {
    startY += step;
  }

  painter.setPen(QPen(QColor("#e1e1dc"), 1));
  for (double x = startX; x < width(); x += step) {
    painter.drawLine(QPointF{x, 0.0}, QPointF{x, static_cast<double>(height())});
  }
  for (double y = startY; y < height(); y += step) {
    painter.drawLine(QPointF{0.0, y}, QPointF{static_cast<double>(width()), y});
  }

  painter.setPen(QPen(QColor("#325f8f"), 2));
  painter.drawLine(sceneToScreen({0.0, 0.0}), sceneToScreen({160.0, 0.0}));
  painter.drawLine(sceneToScreen({160.0, 0.0}), sceneToScreen({160.0, 80.0}));
  painter.drawRect(QRectF{sceneToScreen({30.0, 30.0}), QSizeF{80.0 * zoom_, 40.0 * zoom_}});

  if (hasSelection_) {
    const auto point = sceneToScreen(selectedScenePoint_);
    painter.setPen(QPen(QColor("#c8553d"), 2));
    painter.drawRect(QRectF{point.x() - 5.0, point.y() - 5.0, 10.0, 10.0});
  }
}

void SchematicViewWidget::wheelEvent(QWheelEvent* event) {
  const auto cursorScene = screenToScene(event->position());
  const double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
  zoom_ *= factor;
  if (zoom_ < 0.1) {
    zoom_ = 0.1;
  }
  if (zoom_ > 8.0) {
    zoom_ = 8.0;
  }
  pan_ = event->position() - cursorScene * zoom_;
  update();
}

QPointF SchematicViewWidget::sceneToScreen(QPointF point) const {
  return point * zoom_ + pan_;
}

QPointF SchematicViewWidget::screenToScene(QPointF point) const {
  return (point - pan_) / zoom_;
}

}  // namespace aurora::ui
