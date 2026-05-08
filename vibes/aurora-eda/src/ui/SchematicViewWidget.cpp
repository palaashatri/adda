#include "ui/SchematicViewWidget.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "schematic/SchDocument.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace aurora::ui {

SchematicViewWidget::SchematicViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 300);
  setMouseTracking(true);
}

void SchematicViewWidget::setDocument(const schematic::SchDocument* doc, const db::DbCellLib* lib,
                                     double dbuPerMicron) {
  doc_ = doc;
  lib_ = lib;
  dbuPerMicron_ = (dbuPerMicron > 0.0) ? dbuPerMicron : 1000.0;
  update();
}

void SchematicViewWidget::fitView() {
  zoom_ = 50.0;
  pan_ = {40.0, 40.0};
  update();
}

double SchematicViewWidget::dbuToScene(long long dbu) const {
  return static_cast<double>(dbu) / dbuPerMicron_;
}

QPointF SchematicViewWidget::sceneToScreen(QPointF p) const {
  return p * zoom_ + pan_;
}

QPointF SchematicViewWidget::screenToScene(QPointF p) const {
  return (p - pan_) / zoom_;
}

void SchematicViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    pan_ += event->position() - lastMousePos_;
    lastMousePos_ = event->position();
    update();
  }
  emit coordinatesChanged(screenToScene(event->position()));
}

void SchematicViewWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = true;
    lastMousePos_ = event->position();
    setCursor(Qt::ClosedHandCursor);
    return;
  }
  if (event->button() == Qt::LeftButton) {
    selectedScene_ = screenToScene(event->position());
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

void SchematicViewWidget::wheelEvent(QWheelEvent* event) {
  const auto cursor = screenToScene(event->position());
  const double factor = event->angleDelta().y() > 0 ? 1.15 : (1.0 / 1.15);
  zoom_ = std::clamp(zoom_ * factor, 0.01, 500.0);
  pan_ = event->position() - cursor * zoom_;
  update();
}

void SchematicViewWidget::paintGrid(QPainter& painter) const {
  const double step = std::max(4.0, 20.0 * zoom_);
  double sx = std::fmod(pan_.x(), step);
  double sy = std::fmod(pan_.y(), step);
  if (sx < 0.0) sx += step;
  if (sy < 0.0) sy += step;
  painter.setPen(QPen(QColor("#e1e1dc"), 1));
  for (double x = sx; x < width(); x += step)
    painter.drawLine(QPointF{x, 0.0}, QPointF{x, (double)height()});
  for (double y = sy; y < height(); y += step)
    painter.drawLine(QPointF{0.0, y}, QPointF{(double)width(), y});
}

void SchematicViewWidget::paintSymbol(QPainter& painter, const db::DbView& view, long long dx,
                                    long long dy) const {
  painter.setPen(QPen(QColor("#a02020"), std::max(1.0, 1.0 * zoom_)));
  painter.setBrush(Qt::NoBrush);

  for (const auto shapeId : view.shapeIds()) {
    const auto* shape = view.findShape(shapeId);
    if (!shape) continue;

    switch (shape->kind()) {
      case db::DbShapeKind::Rect: {
        const auto& box = static_cast<const db::DbRect*>(shape)->box();
        const QRectF r{dbuToScene(box.left() + dx), dbuToScene(box.bottom() + dy),
                       dbuToScene(box.width()), dbuToScene(box.height())};
        painter.drawRect(r.left() * zoom_ + pan_.x(), r.top() * zoom_ + pan_.y(),
                         r.width() * zoom_, r.height() * zoom_);
        break;
      }
      case db::DbShapeKind::Polygon: {
        const auto& poly = static_cast<const db::DbPolygon*>(shape)->polygon();
        if (poly.empty()) break;
        QPolygonF qpoly;
        for (const auto& pt : poly.points())
          qpoly << sceneToScreen({dbuToScene(pt.x + dx), dbuToScene(pt.y + dy)});
        painter.drawPolygon(qpoly);
        break;
      }
      case db::DbShapeKind::Path: {
        const auto& path = static_cast<const db::DbPath*>(shape)->path();
        if (path.empty()) break;
        QPainterPath qpath;
        const auto& pts = path.points();
        qpath.moveTo(sceneToScreen({dbuToScene(pts[0].x + dx), dbuToScene(pts[0].y + dy)}));
        for (std::size_t i = 1; i < pts.size(); ++i)
          qpath.lineTo(sceneToScreen({dbuToScene(pts[i].x + dx), dbuToScene(pts[i].y + dy)}));
        painter.drawPath(qpath);
        break;
      }
      case db::DbShapeKind::Text: {
        const auto* text = static_cast<const db::DbText*>(shape);
        const auto sp =
            sceneToScreen({dbuToScene(text->origin().x + dx), dbuToScene(text->origin().y + dy)});
        painter.drawText(sp, QString::fromStdString(text->text()));
        break;
      }
    }
  }

  // Draw pins as small squares
  for (const auto pinId : view.pinIds()) {
    const auto* pin = view.findPin(pinId);
    if (!pin) continue;
    for (const auto shapeId : pin->shapeIds()) {
      if (const auto* s = view.findShape(shapeId)) {
        if (s->kind() == db::DbShapeKind::Rect) {
          const auto& b = static_cast<const db::DbRect*>(s)->box();
          const auto center = b.center();
          const auto sp = sceneToScreen({dbuToScene(center.x + dx), dbuToScene(center.y + dy)});
          painter.fillRect(QRectF{sp.x() - 2, sp.y() - 2, 4, 4}, QColor("#a02020"));
        }
      }
    }
  }
}

void SchematicViewWidget::paintDocument(QPainter& painter) const {
  if (doc_ == nullptr) {
    // Placeholder schematic
    painter.setPen(QPen(QColor("#325f8f"), 2));
    painter.drawLine(sceneToScreen({0.0, 0.0}), sceneToScreen({160.0, 0.0}));
    painter.drawLine(sceneToScreen({160.0, 0.0}), sceneToScreen({160.0, 80.0}));
    painter.drawRect(QRectF{sceneToScreen({30.0, 30.0}), QSizeF{80.0 * zoom_, 40.0 * zoom_}});
    return;
  }

  const auto& view = doc_->view();

  // Draw instances
  for (const auto instId : view.instanceIds()) {
    const auto* inst = view.findInstance(instId);
    if (!inst) continue;
    if (lib_) {
      if (const auto* masterCell = lib_->findCellById(inst->masterCellId())) {
        if (const auto* symbolView = masterCell->findView(db::DbViewType::Symbol)) {
          paintSymbol(painter, *symbolView, inst->transform().dx, inst->transform().dy);
        }
      }
    }
    // Draw instance name
    const auto sp =
        sceneToScreen({dbuToScene(inst->transform().dx), dbuToScene(inst->transform().dy + 5000)});
    painter.setPen(QColor("#202020"));
    painter.drawText(sp, QString::fromStdString(inst->name()));
  }

  // Draw wires
  painter.setPen(QPen(QColor("#325f8f"), std::max(1.0, 1.5 * zoom_)));
  for (const auto& wire : doc_->wires()) {
    const auto& pts = wire.points();
    if (pts.size() < 2) continue;
    for (std::size_t i = 1; i < pts.size(); ++i) {
      painter.drawLine(
          sceneToScreen({dbuToScene(pts[i - 1].x), dbuToScene(pts[i - 1].y)}),
          sceneToScreen({dbuToScene(pts[i].x),     dbuToScene(pts[i].y)}));
    }
    // Junction dot at each vertex except endpoints
    painter.setBrush(QColor("#325f8f"));
    painter.setPen(Qt::NoPen);
    for (std::size_t i = 1; i + 1 < pts.size(); ++i) {
      const auto sp = sceneToScreen({dbuToScene(pts[i].x), dbuToScene(pts[i].y)});
      const double r = std::max(2.0, 3.0 * zoom_);
      painter.drawEllipse(sp, r, r);
    }
    painter.setPen(QPen(QColor("#325f8f"), std::max(1.0, 1.5 * zoom_)));
    painter.setBrush(Qt::NoBrush);
  }

  // Origin marker
  const auto orig = sceneToScreen({0.0, 0.0});
  painter.setPen(QPen(QColor("#c8c8c0"), 1));
  const double m = 6.0;
  painter.drawLine(QPointF{orig.x() - m, orig.y()}, QPointF{orig.x() + m, orig.y()});
  painter.drawLine(QPointF{orig.x(), orig.y() - m}, QPointF{orig.x(), orig.y() + m});
}

void SchematicViewWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.fillRect(rect(), QColor("#fbfbf8"));
  painter.setRenderHint(QPainter::Antialiasing, true);

  paintGrid(painter);
  paintDocument(painter);

  if (hasSelection_) {
    const auto pt = sceneToScreen(selectedScene_);
    painter.setPen(QPen(QColor("#c8553d"), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF{pt.x() - 5.0, pt.y() - 5.0, 10.0, 10.0});
  }
}

}  // namespace aurora::ui
