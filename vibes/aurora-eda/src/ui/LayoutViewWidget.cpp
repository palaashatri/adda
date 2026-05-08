#include "ui/LayoutViewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace aurora::ui {

LayoutViewWidget::LayoutViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 300);
  setMouseTracking(true);
}

void LayoutViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    const auto delta = event->position() - lastMousePosition_;
    pan_ += delta;
    lastMousePosition_ = event->position();
    update();
  }
}

void LayoutViewWidget::mousePressEvent(QMouseEvent* event) {
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

void LayoutViewWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = false;
    unsetCursor();
  }
}

void LayoutViewWidget::paintEvent(QPaintEvent* event) {
  (void)event;

  QPainter painter(this);
  painter.fillRect(rect(), QColor("#111418"));
  painter.setRenderHint(QPainter::Antialiasing, false);

  const double step = std::max(4.0, 25.0 * zoom_);
  auto startX = std::fmod(pan_.x(), step);
  auto startY = std::fmod(pan_.y(), step);
  if (startX < 0.0) {
    startX += step;
  }
  if (startY < 0.0) {
    startY += step;
  }

  painter.setPen(QPen(QColor("#252b31"), 1));
  for (double x = startX; x < width(); x += step) {
    painter.drawLine(QPointF{x, 0.0}, QPointF{x, static_cast<double>(height())});
  }
  for (double y = startY; y < height(); y += step) {
    painter.drawLine(QPointF{0.0, y}, QPointF{static_cast<double>(width()), y});
  }

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor("#3fbf7f"));
  painter.drawRect(sceneRectToScreen(QRectF{10.0, 10.0, 180.0, 88.0}));
  painter.setBrush(QColor("#d9b34c"));
  painter.drawRect(sceneRectToScreen(QRectF{50.0, 50.0, 180.0, 88.0}));

  if (hasSelection_) {
    const auto point = sceneToScreen(selectedScenePoint_);
    painter.setPen(QPen(QColor("#f0f3f5"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF{point.x() - 6.0, point.y() - 6.0, 12.0, 12.0});
  }
}

void LayoutViewWidget::wheelEvent(QWheelEvent* event) {
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

QPointF LayoutViewWidget::sceneToScreen(QPointF point) const {
  return point * zoom_ + pan_;
}

QPointF LayoutViewWidget::screenToScene(QPointF point) const {
  return (point - pan_) / zoom_;
}

QRectF LayoutViewWidget::sceneRectToScreen(const QRectF& rect) const {
  return {sceneToScreen(rect.topLeft()), rect.size() * zoom_};
}

}  // namespace aurora::ui
