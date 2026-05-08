#include "ui/WaveformViewWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>

namespace aurora::ui {

WaveformViewWidget::WaveformViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 250);
  setMouseTracking(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

void WaveformViewWidget::addTrace(std::string name, std::vector<double> x, std::vector<double> y,
                                   QColor color) {
  traces_.push_back({std::move(name), std::move(x), std::move(y), color});
  computeBounds();
  fitView();
  update();
}

void WaveformViewWidget::clearTraces() {
  traces_.clear();
  update();
}

void WaveformViewWidget::setAxisLabels(const QString& xLabel, const QString& yLabel) {
  xLabel_ = xLabel;
  yLabel_ = yLabel;
  update();
}

void WaveformViewWidget::computeBounds() {
  xMin_ = std::numeric_limits<double>::max();
  xMax_ = std::numeric_limits<double>::lowest();
  yMin_ = std::numeric_limits<double>::max();
  yMax_ = std::numeric_limits<double>::lowest();
  for (const auto& t : traces_) {
    for (const auto v : t.x) { xMin_ = std::min(xMin_, v); xMax_ = std::max(xMax_, v); }
    for (const auto v : t.y) { yMin_ = std::min(yMin_, v); yMax_ = std::max(yMax_, v); }
  }
  if (xMin_ >= xMax_) { xMin_ = 0.0; xMax_ = 1.0; }
  if (yMin_ >= yMax_) { yMin_ -= 0.5; yMax_ += 0.5; }
}

void WaveformViewWidget::fitView() {
  const double padX = (xMax_ - xMin_) * 0.05;
  const double padY = (yMax_ - yMin_) * 0.1;
  viewXMin_ = xMin_ - padX;
  viewXMax_ = xMax_ + padX;
  viewYMin_ = yMin_ - padY;
  viewYMax_ = yMax_ + padY;
}

double WaveformViewWidget::toScreenX(double x) const {
  const double span = viewXMax_ - viewXMin_;
  const double pw = width() - kMarginL - kMarginR;
  return kMarginL + (x - viewXMin_) / span * pw;
}

double WaveformViewWidget::toScreenY(double y) const {
  const double span = viewYMax_ - viewYMin_;
  const double ph = height() - kMarginT - kMarginB;
  return kMarginT + (1.0 - (y - viewYMin_) / span) * ph;
}

void WaveformViewWidget::paintAxes(QPainter& p) const {
  const int pw = width() - kMarginL - kMarginR;
  const int ph = height() - kMarginT - kMarginB;

  // Plot area border
  p.setPen(QPen(QColor("#505a60"), 1));
  p.drawRect(kMarginL, kMarginT, pw, ph);

  // Grid
  const int nX = 5, nY = 4;
  p.setPen(QPen(QColor("#2a3038"), 1));
  for (int i = 1; i < nX; ++i) {
    const double sx = kMarginL + i * pw / nX;
    p.drawLine(QPointF{sx, (double)kMarginT}, QPointF{sx, (double)(height() - kMarginB)});
  }
  for (int i = 1; i < nY; ++i) {
    const double sy = kMarginT + i * ph / nY;
    p.drawLine(QPointF{(double)kMarginL, sy}, QPointF{(double)(width() - kMarginR), sy});
  }

  // Tick labels
  p.setPen(QColor("#9aaab5"));
  p.setFont(QFont("monospace", 8));
  const double xRange = viewXMax_ - viewXMin_;
  const double yRange = viewYMax_ - viewYMin_;
  for (int i = 0; i <= nX; ++i) {
    const double val = viewXMin_ + i * xRange / nX;
    const double sx  = kMarginL + i * pw / nX;
    p.drawText(QRectF{sx - 30, (double)(height() - kMarginB + 4), 60, 16},
               Qt::AlignCenter, QString::number(val, 'g', 4));
  }
  for (int i = 0; i <= nY; ++i) {
    const double val = viewYMax_ - i * yRange / nY;
    const double sy  = kMarginT + i * ph / nY;
    p.drawText(QRectF{0, sy - 8, kMarginL - 4, 16},
               Qt::AlignRight | Qt::AlignVCenter, QString::number(val, 'g', 4));
  }

  // Axis labels
  p.setPen(QColor("#c8d0d8"));
  p.setFont(QFont("monospace", 9));
  p.drawText(QRectF{(double)kMarginL, (double)(height() - 16), (double)pw, 16.0},
             Qt::AlignCenter, xLabel_);
  p.save();
  p.translate(14, kMarginT + ph / 2.0);
  p.rotate(-90);
  p.drawText(QRectF{-(double)(ph / 2), -10, (double)ph, 20.0}, Qt::AlignCenter, yLabel_);
  p.restore();
}

void WaveformViewWidget::paintTraces(QPainter& p) const {
  p.setClipRect(kMarginL, kMarginT, width() - kMarginL - kMarginR, height() - kMarginT - kMarginB);
  for (const auto& trace : traces_) {
    if (trace.x.size() < 2) continue;
    p.setPen(QPen(trace.color, 1.5));
    QPolygonF poly;
    poly.reserve((int)trace.x.size());
    for (std::size_t i = 0; i < trace.x.size(); ++i)
      poly << QPointF{toScreenX(trace.x[i]), toScreenY(trace.y[i])};
    p.drawPolyline(poly);
  }
  p.setClipping(false);
}

void WaveformViewWidget::paintLegend(QPainter& p) const {
  if (traces_.empty()) return;
  int x = kMarginL + 8;
  int y = kMarginT + 8;
  for (const auto& trace : traces_) {
    p.setPen(trace.color);
    p.drawLine(x, y + 6, x + 20, y + 6);
    p.setPen(QColor("#c8d0d8"));
    p.drawText(x + 24, y + 10, QString::fromStdString(trace.name));
    y += 18;
  }
}

void WaveformViewWidget::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.fillRect(rect(), QColor("#0e1318"));

  if (traces_.empty()) {
    p.setPen(QColor("#505a60"));
    p.drawText(rect(), Qt::AlignCenter, "No simulation data.\nRun simulation first.");
    return;
  }

  paintAxes(p);
  paintTraces(p);
  paintLegend(p);
}

void WaveformViewWidget::wheelEvent(QWheelEvent* event) {
  const double factor = event->angleDelta().y() > 0 ? 0.85 : 1.15;
  const double cx = viewXMin_ + (event->position().x() - kMarginL) /
      (width() - kMarginL - kMarginR) * (viewXMax_ - viewXMin_);
  const double cy = viewYMax_ - (event->position().y() - kMarginT) /
      (height() - kMarginT - kMarginB) * (viewYMax_ - viewYMin_);
  viewXMin_ = cx - (cx - viewXMin_) * factor;
  viewXMax_ = cx + (viewXMax_ - cx) * factor;
  viewYMin_ = cy - (cy - viewYMin_) * factor;
  viewYMax_ = cy + (viewYMax_ - cy) * factor;
  update();
}

void WaveformViewWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
    panning_ = true;
    lastMousePos_ = event->position();
  }
}

void WaveformViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (panning_) {
    const double dx = event->position().x() - lastMousePos_.x();
    const double dy = event->position().y() - lastMousePos_.y();
    const double pw = width() - kMarginL - kMarginR;
    const double ph = height() - kMarginT - kMarginB;
    const double dxScene = -dx / pw * (viewXMax_ - viewXMin_);
    const double dyScene =  dy / ph * (viewYMax_ - viewYMin_);
    viewXMin_ += dxScene; viewXMax_ += dxScene;
    viewYMin_ += dyScene; viewYMax_ += dyScene;
    lastMousePos_ = event->position();
    update();
  }
}

void WaveformViewWidget::mouseReleaseEvent(QMouseEvent*) { panning_ = false; }

}  // namespace aurora::ui
