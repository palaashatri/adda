#include "ui/LayoutViewWidget.h"

#include "db/DbCellLib.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "layout/LayDocument.h"
#include "layout/LayEditorController.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace aurora::ui {

LayoutViewWidget::LayoutViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 300);
  setMouseTracking(true);
}

void LayoutViewWidget::setDocument(const layout::LayDocument* doc, const db::DbCellLib* lib,
                                   double dbuPerMicron) {
  doc_ = doc;
  lib_ = lib;
  dbuPerMicron_ = (dbuPerMicron > 0.0) ? dbuPerMicron : 1000.0;
  update();
}

void LayoutViewWidget::setController(layout::LayEditorController* ctrl) {
  ctrl_ = ctrl;
}

void LayoutViewWidget::fitView() {
  zoom_ = 50.0;
  pan_ = {64.0, 64.0};
  update();
}

double LayoutViewWidget::dbuToScene(long long dbu) const {
  return static_cast<double>(dbu) / dbuPerMicron_;
}

QPointF LayoutViewWidget::sceneToScreen(QPointF p) const {
  return p * zoom_ + pan_;
}

QPointF LayoutViewWidget::screenToScene(QPointF p) const {
  return (p - pan_) / zoom_;
}

QRectF LayoutViewWidget::sceneRectToScreen(const QRectF& r) const {
  return {sceneToScreen(r.topLeft()), r.size() * zoom_};
}

void LayoutViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    pan_ += event->position() - lastMousePos_;
    lastMousePos_ = event->position();
    update();
  }
  const auto scenePt = screenToScene(event->position());
  if (ctrl_) {
    ctrl_->mouseMove({static_cast<geom::DbUnit>(scenePt.x() * dbuPerMicron_),
                      static_cast<geom::DbUnit>(scenePt.y() * dbuPerMicron_)});
  }
  emit coordinatesChanged(scenePt);
}

void LayoutViewWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = true;
    lastMousePos_ = event->position();
    setCursor(Qt::ClosedHandCursor);
    return;
  }
  const auto scenePt = screenToScene(event->position());
  if (event->button() == Qt::LeftButton) {
    selectedScene_ = scenePt;
    hasSelection_ = true;
    if (ctrl_) {
      ctrl_->mousePress({static_cast<geom::DbUnit>(scenePt.x() * dbuPerMicron_),
                         static_cast<geom::DbUnit>(scenePt.y() * dbuPerMicron_)});
    }
    update();
  }
}

void LayoutViewWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = false;
    unsetCursor();
  }
  const auto scenePt = screenToScene(event->position());
  if (event->button() == Qt::LeftButton) {
    if (ctrl_) {
      ctrl_->mouseRelease({static_cast<geom::DbUnit>(scenePt.x() * dbuPerMicron_),
                           static_cast<geom::DbUnit>(scenePt.y() * dbuPerMicron_)});
      update();
    }
  }
}

void LayoutViewWidget::wheelEvent(QWheelEvent* event) {
  const auto cursor = screenToScene(event->position());
  const double factor = event->angleDelta().y() > 0 ? 1.15 : (1.0 / 1.15);
  zoom_ = std::clamp(zoom_ * factor, 0.01, 500.0);
  pan_ = event->position() - cursor * zoom_;
  update();
}

void LayoutViewWidget::paintGrid(QPainter& painter) const {
  const double step = std::max(4.0, 25.0 * zoom_);
  double sx = std::fmod(pan_.x(), step);
  double sy = std::fmod(pan_.y(), step);
  if (sx < 0.0) sx += step;
  if (sy < 0.0) sy += step;

  painter.setPen(QPen(QColor("#252b31"), 1));
  for (double x = sx; x < width(); x += step)
    painter.drawLine(QPointF{x, 0.0}, QPointF{x, (double)height()});
  for (double y = sy; y < height(); y += step)
    painter.drawLine(QPointF{0.0, y}, QPointF{(double)width(), y});
}

void LayoutViewWidget::setLayerVisible(db::DbId layerId, bool visible) {
  if (visible) {
    hiddenLayers_.erase(layerId);
  } else {
    hiddenLayers_.insert(layerId);
  }
  update();
}

void LayoutViewWidget::paintView(QPainter& painter, const db::DbView& view, long long dx, long long dy) const {
  for (const auto shapeId : view.shapeIds()) {
    const auto* shape = view.findShape(shapeId);
    if (shape == nullptr) continue;

    if (hiddenLayers_.count(shape->layerId())) continue;

    // Resolve layer color
    QColor fill("#808080");
    if (lib_ != nullptr) {
      if (const auto* layer = lib_->findLayer(shape->layerId())) {
        fill = QColor(QString::fromStdString(layer->color()));
        fill.setAlpha(180);
      }
    }

    painter.setPen(QPen(fill.lighter(140), 1));
    painter.setBrush(fill);

    switch (shape->kind()) {
      case db::DbShapeKind::Rect: {
        const auto& box = static_cast<const db::DbRect*>(shape)->box();
        const QRectF r{dbuToScene(box.left() + dx), dbuToScene(box.bottom() + dy),
                        dbuToScene(box.width()), dbuToScene(box.height())};
        painter.drawRect(sceneRectToScreen(r));
        break;
      }
      case db::DbShapeKind::Polygon: {
        const auto& poly = static_cast<const db::DbPolygon*>(shape)->polygon();
        if (poly.empty()) break;
        QPolygonF qpoly;
        qpoly.reserve((int)poly.points().size());
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
        const double w = dbuToScene(path.width()) * zoom_;
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(fill, std::max(1.0, w), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter.drawPath(qpath);
        break;
      }
      case db::DbShapeKind::Text: {
        const auto* text = static_cast<const db::DbText*>(shape);
        const auto sp = sceneToScreen({dbuToScene(text->origin().x + dx), dbuToScene(text->origin().y + dy)});
        painter.setPen(fill.lighter(160));
        painter.drawText(sp, QString::fromStdString(text->text()));
        break;
      }
    }
  }

  for (const auto instId : view.instanceIds()) {
    const auto* inst = view.findInstance(instId);
    if (!inst) continue;
    if (lib_) {
      if (const auto* masterCell = lib_->findCellById(inst->masterCellId())) {
        if (const auto* masterView = masterCell->findView(db::DbViewType::Layout)) {
          paintView(painter, *masterView, dx + inst->transform().dx, dy + inst->transform().dy);
        }
      }
    }
  }
}

void LayoutViewWidget::paintDocument(QPainter& painter) const {
  if (doc_ == nullptr) {
    // Placeholder
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#3fbf7f80"));
    painter.drawRect(sceneRectToScreen({10.0, 10.0, 180.0, 88.0}));
    painter.setBrush(QColor("#d9b34c80"));
    painter.drawRect(sceneRectToScreen({50.0, 50.0, 180.0, 88.0}));
    return;
  }

  paintView(painter, doc_->view(), 0, 0);

  // Origin cross-hair
  const auto orig = sceneToScreen({0.0, 0.0});
  painter.setPen(QPen(QColor("#404850"), 1));
  painter.drawLine(QPointF{orig.x(), 0.0}, QPointF{orig.x(), (double)height()});
  painter.drawLine(QPointF{0.0, orig.y()}, QPointF{(double)width(), orig.y()});
}

void LayoutViewWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.fillRect(rect(), QColor("#111418"));
  painter.setRenderHint(QPainter::Antialiasing, false);

  paintGrid(painter);
  paintDocument(painter);

  if (hasSelection_) {
    const auto pt = sceneToScreen(selectedScene_);
    painter.setPen(QPen(QColor("#f0f3f5"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF{pt.x() - 6.0, pt.y() - 6.0, 12.0, 12.0});
  }
}

}  // namespace aurora::ui
