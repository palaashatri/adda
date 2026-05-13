#include "ui/SchematicViewWidget.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "schematic/SchDocument.h"
#include "schematic/SchEditorController.h"
#include "db/DbConstraint.h"
#include "schematic/SchToolProbe.h"
#include "schematic/SchToolStimulus.h"
#include "schematic/SchToolWire.h"
#include "schematic/SchToolSelect.h"
#include "schematic/SchToolInstance.h"
#include "schematic/SchToolLabel.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

#include <QFont>

#include <algorithm>
#include <cmath>

namespace aurora::ui {

SchematicViewWidget::SchematicViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 300);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}

void SchematicViewWidget::setDocument(const schematic::SchDocument* doc, const db::DbCellLib* lib,
                                      double dbuPerMicron) {
  doc_ = doc;
  lib_ = lib;
  dbuPerMicron_ = (dbuPerMicron > 0.0) ? dbuPerMicron : 1000.0;
  update();
}

void SchematicViewWidget::setController(schematic::SchEditorController* ctrl) {
  ctrl_ = ctrl;
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

geom::GeomPoint SchematicViewWidget::toDbPoint(QPointF screenPt) const {
  const auto scene = screenToScene(screenPt);
  return {static_cast<geom::DbUnit>(scene.x() * dbuPerMicron_),
          static_cast<geom::DbUnit>(scene.y() * dbuPerMicron_)};
}

schematic::SchKeyEvent SchematicViewWidget::mapKey(int qtKey) {
  switch (qtKey) {
    case Qt::Key_Escape: return schematic::SchKeyEvent::Escape;
    case Qt::Key_Return:
    case Qt::Key_Enter:  return schematic::SchKeyEvent::Enter;
    case Qt::Key_Delete:
    case Qt::Key_Backspace: return schematic::SchKeyEvent::Delete;
    default: return schematic::SchKeyEvent::Other;
  }
}

void SchematicViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    pan_ += event->position() - lastMousePos_;
    lastMousePos_ = event->position();
    update();
  }
  if (ctrl_) {
    ctrl_->mouseMove(toDbPoint(event->position()));
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
    if (ctrl_) {
      ctrl_->mousePress(toDbPoint(event->position()));
      update();
    } else {
      update();
    }
  }
  if (event->button() == Qt::RightButton && ctrl_) {
    ctrl_->keyPress(schematic::SchKeyEvent::Escape);
    update();
  }
}

void SchematicViewWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton) {
    panning_ = false;
    unsetCursor();
  }
  const auto scenePt = screenToScene(event->position());
  if (event->button() == Qt::LeftButton && ctrl_) {
    ctrl_->mouseRelease({static_cast<geom::DbUnit>(scenePt.x() * dbuPerMicron_),
                         static_cast<geom::DbUnit>(scenePt.y() * dbuPerMicron_)});
    update();
    emit selectionChanged();
  }
}

void SchematicViewWidget::keyPressEvent(QKeyEvent* event) {
  if (ctrl_) {
    ctrl_->keyPress(mapKey(event->key()));
    update();
  }
  QWidget::keyPressEvent(event);
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
        const auto tl = sceneToScreen({dbuToScene(box.left() + dx), dbuToScene(box.bottom() + dy)});
        painter.drawRect(QRectF{tl, QSizeF{dbuToScene(box.width()) * zoom_,
                                           dbuToScene(box.height()) * zoom_}});
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
          const geom::GeomPoint center{(b.left() + b.right()) / 2, (b.bottom() + b.top()) / 2};
          const auto sp = sceneToScreen({dbuToScene(center.x + dx), dbuToScene(center.y + dy)});
          painter.fillRect(QRectF{sp.x() - 2, sp.y() - 2, 4, 4}, QColor("#a02020"));
        }
      }
    }
  }
}

void SchematicViewWidget::paintDocument(QPainter& painter) const {
  if (doc_ == nullptr) {
    painter.setPen(QPen(QColor("#325f8f"), 2));
    painter.drawLine(sceneToScreen({0.0, 0.0}), sceneToScreen({160.0, 0.0}));
    painter.drawLine(sceneToScreen({160.0, 0.0}), sceneToScreen({160.0, 80.0}));
    painter.drawRect(QRectF{sceneToScreen({30.0, 30.0}), QSizeF{80.0 * zoom_, 40.0 * zoom_}});
    return;
  }

  const auto& view = doc_->view();

  // Selected instances highlight
  std::set<db::DbId> selectedIds;
  if (ctrl_) {
    if (const auto* sel = dynamic_cast<const schematic::SchToolSelect*>(ctrl_->activeTool())) {
      selectedIds = sel->selectedInstances();
    }
  }

  // Draw instances
  for (const auto instId : view.instanceIds()) {
    const auto* inst = view.findInstance(instId);
    if (!inst) continue;

    // Cross-probe highlight: same master cell
    if (crossProbeCellId_ != db::kInvalidId && inst->masterCellId() == crossProbeCellId_) {
      painter.setPen(QPen(QColor("#00ffcc"), 2));
      const auto sp =
          sceneToScreen({dbuToScene(inst->transform().dx), dbuToScene(inst->transform().dy)});
      painter.drawEllipse(sp, 12, 12);
    }

    // Highlight if selected
    if (selectedIds.count(instId)) {
      painter.setPen(QPen(QColor("#ffcc00"), 2));
      const auto sp =
          sceneToScreen({dbuToScene(inst->transform().dx), dbuToScene(inst->transform().dy)});
      painter.drawRect(QRectF{sp.x() - 8, sp.y() - 8, 16, 16});
    }

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

    // Draw pin labels (B4)
    painter.setFont(QFont("sans-serif", 7));
    painter.setPen(QColor("#6060a0"));
    if (lib_) {
      if (const auto* masterCell = lib_->findCellById(inst->masterCellId())) {
        if (const auto* symbolView = masterCell->findView(db::DbViewType::Symbol)) {
          for (const auto mpid : symbolView->pinIds()) {
            const auto* mpin = symbolView->findPin(mpid);
            if (!mpin) continue;
            // Find connected net name
            auto instPins = view.findInstancePins(instId);
            std::string netName;
            for (const auto* ipin : instPins) {
              if (ipin->name() == mpin->name()) {
                if (ipin->netId() != db::kInvalidId) {
                  const auto* n = view.findNet(ipin->netId());
                  if (n) netName = n->name();
                }
                break;
              }
            }
            // Place label near the pin position in the symbol
            // For now, draw pin name stacked vertically at instance origin
            const auto px = inst->transform().dx - 8000;
            const auto py = inst->transform().dy + static_cast<long long>(mpin->id() * 3000);
            const auto psp = sceneToScreen({dbuToScene(px), dbuToScene(py)});
            const QString pLabel = QString("%1:%2").arg(
                QString::fromStdString(mpin->name()),
                QString::fromStdString(netName));
            painter.drawText(psp, pLabel);
          }
        }
      }
    }
  }

  // Draw wires (bus and regular)
  for (const auto& wire : doc_->wires()) {
    const auto& pts = wire.points();
    if (pts.size() < 2) continue;
    const double penW = wire.isBus() ? std::max(2.0, 4.0 * zoom_) : std::max(1.0, 1.5 * zoom_);
    painter.setPen(QPen(wire.isBus() ? QColor("#004080") : QColor("#325f8f"), penW));
    for (std::size_t i = 1; i < pts.size(); ++i) {
      painter.drawLine(
          sceneToScreen({dbuToScene(pts[i - 1].x), dbuToScene(pts[i - 1].y)}),
          sceneToScreen({dbuToScene(pts[i].x),     dbuToScene(pts[i].y)}));
    }

    // Bus slash marks
    if (wire.isBus()) {
      painter.setPen(QPen(QColor("#004080"), 2));
      for (std::size_t i = 1; i < pts.size(); ++i) {
        const auto a = sceneToScreen({dbuToScene(pts[i - 1].x), dbuToScene(pts[i - 1].y)});
        const auto b = sceneToScreen({dbuToScene(pts[i].x), dbuToScene(pts[i].y)});
        const auto mid = (a + b) / 2.0;
        painter.drawLine(mid + QPointF{-4, -4}, mid + QPointF{4, 4});
      }
    }

    painter.setBrush(wire.isBus() ? QColor("#004080") : QColor("#325f8f"));
    painter.setPen(Qt::NoPen);
    for (std::size_t i = 1; i + 1 < pts.size(); ++i) {
      const auto sp = sceneToScreen({dbuToScene(pts[i].x), dbuToScene(pts[i].y)});
      const double r = std::max(2.0, 3.0 * zoom_);
      painter.drawEllipse(sp, r, r);
    }
    painter.setPen(QPen(wire.isBus() ? QColor("#004080") : QColor("#325f8f"), penW));
    painter.setBrush(Qt::NoBrush);
  }

  // Draw net labels
  painter.setFont(QFont("sans-serif", 9));
  for (const auto& label : doc_->netLabels()) {
    const auto* net = doc_->view().findNet(label.netId);
    if (!net) continue;
    const auto sp = sceneToScreen({dbuToScene(label.position.x), dbuToScene(label.position.y)});
    const QString text = QString::fromStdString(net->name());
    const QRectF br = painter.fontMetrics().boundingRect(text);
    // Background pill
    const QRectF bg{sp.x() - 4, sp.y() - br.height() + 2, br.width() + 8, br.height() + 2};
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#ffffcc"));
    painter.drawRoundedRect(bg, 3, 3);
    // Text
    painter.setPen(QColor("#004000"));
    painter.setBrush(Qt::NoBrush);
    painter.drawText(sp + QPointF{0, 2}, text);
  }

  // Draw DC operating point annotation (B10)
  if (!dcAnnotation_.empty()) {
    painter.setFont(QFont("sans-serif", 9, QFont::Bold));
    for (const auto& wire : doc_->wires()) {
      const auto* net = doc_->view().findNet(wire.netId());
      if (!net) continue;
      auto it = dcAnnotation_.find(net->name());
      if (it == dcAnnotation_.end()) continue;
      // Find center of first segment
      const auto& pts = wire.points();
      if (pts.size() < 2) continue;
      const geom::GeomPoint mid{(pts[0].x + pts[1].x) / 2, (pts[0].y + pts[1].y) / 2};
      const auto sp = sceneToScreen({dbuToScene(mid.x), dbuToScene(mid.y)});
      const QString text = QString("%1 V").arg(it->second, 0, 'f', 3);
      const QRectF br = painter.fontMetrics().boundingRect(text);
      const QRectF bg{sp.x() - 4, sp.y() - 12, br.width() + 8, br.height() + 4};
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor("#004080"));
      painter.drawRoundedRect(bg, 3, 3);
      painter.setPen(QColor("#88ddff"));
      painter.drawText(sp + QPointF{0, 2}, text);
    }
  }

  // Draw stimulus markers
  painter.setFont(QFont("sans-serif", 8));
  for (const auto cid : doc_->view().constraintIds()) {
    const auto* con = doc_->view().findConstraint(cid);
    if (!con) continue;
    const auto& type = con->type();
    bool isStim = (type == "vdc" || type == "idc" || type == "vpulse" || type == "vsin"
                   || type == "vprobe" || type == "iprobe");
    if (!isStim) continue;
    const auto& props = con->properties();
    auto prop = [&](const std::string& k, const std::string& d) -> std::string {
  auto it = props.find(k); return it != props.end() ? it->second : d;
};
auto getX = props.find("x");
    auto getY = props.find("y");
    if (getX == props.end() || getY == props.end()) continue;
    const geom::GeomPoint pos{std::stoll(getX->second), std::stoll(getY->second)};
    const auto sp = sceneToScreen({dbuToScene(pos.x), dbuToScene(pos.y)});

    // Draw marker symbol
    painter.setPen(QPen(QColor("#c04040"), std::max(1.0, 1.5 * zoom_)));
    painter.setBrush(QColor("#c0404020"));

    QString label;
    if (type == "vdc") {
      // Circle with + and - labels
      painter.drawEllipse(sp, 10, 10);
      painter.drawText(sp + QPointF{-6, -14}, "V");
      label = QString("DC=%1").arg(QString::fromStdString(prop("dc", "0")));
    } else if (type == "idc") {
      painter.drawEllipse(sp, 10, 10);
      painter.drawText(sp + QPointF{-4, -14}, "I");
      label = QString("DC=%1").arg(QString::fromStdString(prop("dc", "0")));
    } else if (type == "vpulse") {
      // Pulse symbol
      auto path = QPainterPath();
      path.moveTo(sp + QPointF{-8, 0});
      path.lineTo(sp + QPointF{-4, 0});
      path.lineTo(sp + QPointF{0, -8});
      path.lineTo(sp + QPointF{4, 0});
      path.lineTo(sp + QPointF{8, 0});
      painter.drawPath(path);
      label = QString("PULSE");
    } else if (type == "vsin") {
      // Sine wave
      auto path = QPainterPath();
      const int n = 20;
      for (int i = 0; i <= n; ++i) {
        const double a = 2.0 * 3.14159 * i / n;
        const double sx = sp.x() + (i - n / 2.0) * 0.8;
        const double sy = sp.y() - 8.0 * std::sin(a);
        if (i == 0) path.moveTo(sx, sy);
        else path.lineTo(sx, sy);
      }
      painter.drawPath(path);
      label = QString("SIN");
    } else if (type == "vprobe") {
      // Voltmeter symbol: circle with V
      painter.drawEllipse(sp, 10, 10);
      painter.drawText(sp + QPointF{-4, -14}, "V");
      label = QString("V-Probe");
    } else if (type == "iprobe") {
      // Ammeter symbol: circle with I
      painter.drawEllipse(sp, 10, 10);
      painter.drawText(sp + QPointF{-4, -14}, "I");
      label = QString("I-Probe");
    }

    // Label text below marker
    painter.setPen(QColor("#c04040"));
    painter.drawText(sp + QPointF{-20, 20}, label);
  }

  // Origin marker
  const auto orig = sceneToScreen({0.0, 0.0});
  painter.setPen(QPen(QColor("#c8c8c0"), 1));
  const double m = 6.0;
  painter.drawLine(QPointF{orig.x() - m, orig.y()}, QPointF{orig.x() + m, orig.y()});
  painter.drawLine(QPointF{orig.x(), orig.y() - m}, QPointF{orig.x(), orig.y() + m});
}

void SchematicViewWidget::paintToolOverlay(QPainter& painter) const {
  if (!ctrl_) return;
  const auto* tool = ctrl_->activeTool();
  if (!tool) return;

  // Wire tool: draw ghost line from start to cursor
  if (const auto* wireTool = dynamic_cast<const schematic::SchToolWire*>(tool)) {
    if (wireTool->isDrawing()) {
      const auto start = wireTool->startPoint();
      const auto end   = wireTool->cursor();
      painter.setPen(QPen(QColor("#5080d0"), std::max(1.0, 1.5 * zoom_), Qt::DashLine));
      painter.drawLine(
          sceneToScreen({dbuToScene(start.x), dbuToScene(start.y)}),
          sceneToScreen({dbuToScene(end.x),   dbuToScene(end.y)}));
      // Dot at start
      const auto sp = sceneToScreen({dbuToScene(start.x), dbuToScene(start.y)});
      painter.setBrush(QColor("#5080d0"));
      painter.setPen(Qt::NoPen);
      painter.drawEllipse(sp, 4.0, 4.0);
    }
  }

  // Select tool: rubber-band selection box
  if (const auto* selTool = dynamic_cast<const schematic::SchToolSelect*>(tool)) {
    if (selTool->isRubberBanding()) {
      const auto s = selTool->rubberBandStart();
      const auto e = selTool->rubberBandEnd();
      const auto sp = sceneToScreen({dbuToScene(s.x), dbuToScene(s.y)});
      const auto ep = sceneToScreen({dbuToScene(e.x), dbuToScene(e.y)});
      painter.setPen(QPen(QColor("#00aaff"), 1, Qt::DashLine));
      painter.setBrush(QColor(0, 170, 255, 30));
      painter.drawRect(QRectF{sp, ep}.normalized());
    }
  }

  // Label tool: crosshair cursor at click point
  if (const auto* labelTool = dynamic_cast<const schematic::SchToolLabel*>(tool)) {
    const auto cp = sceneToScreen({dbuToScene(labelTool->cursor().x),
                                   dbuToScene(labelTool->cursor().y)});
    painter.setPen(QPen(QColor("#008000"), 1, Qt::DashLine));
    painter.drawLine(QPointF{cp.x() - 8, cp.y()}, QPointF{cp.x() + 8, cp.y()});
    painter.drawLine(QPointF{cp.x(), cp.y() - 8}, QPointF{cp.x(), cp.y() + 8});
    painter.setPen(QColor("#008000"));
    painter.drawText(cp + QPointF{4, -8}, "label");
  }

  // Instance tool: ghost instance at cursor
  if (const auto* instTool = dynamic_cast<const schematic::SchToolInstance*>(tool)) {
    const auto cp = sceneToScreen({dbuToScene(instTool->cursor().x),
                                   dbuToScene(instTool->cursor().y)});
    painter.setPen(QPen(QColor("#00cc88"), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF{cp.x() - 10, cp.y() - 10, 20, 20});
    painter.setPen(QColor("#00cc88"));
    painter.drawText(cp + QPointF{4, -4}, "inst");
  }
}

void SchematicViewWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.fillRect(rect(), QColor("#fbfbf8"));
  painter.setRenderHint(QPainter::Antialiasing, true);

  paintGrid(painter);
  paintDocument(painter);
  paintToolOverlay(painter);

  if (hasSelection_ && !ctrl_) {
    const auto pt = sceneToScreen(selectedScene_);
    painter.setPen(QPen(QColor("#c8553d"), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF{pt.x() - 5.0, pt.y() - 5.0, 10.0, 10.0});
  }

  // Crosshair cursor (K5)
  if (hasMouseTracking() && !lastMousePos_.isNull()) {
    painter.setPen(QPen(QColor("#d0d0d0"), 1, Qt::DashLine));
    painter.drawLine(QPointF(lastMousePos_.x(), 0),
                     QPointF(lastMousePos_.x(), (double)height()));
    painter.drawLine(QPointF(0, lastMousePos_.y()),
                     QPointF((double)width(), lastMousePos_.y()));
  }
}

}  // namespace aurora::ui
