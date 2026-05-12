#include "ui/WaveformViewWidget.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <sstream>

namespace aurora::ui {

WaveformViewWidget::WaveformViewWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(400, 250);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

void WaveformViewWidget::addTrace(std::string name, std::vector<double> x, std::vector<double> y,
                                   QColor color) {
  // Normalize name: strip leading numbers like "1.800000 " from sweep values
  auto cleanName = name;
  auto space = cleanName.find(' ');
  if (space != std::string::npos) {
    // Keep the full name for sweeps
  }
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
  // Reset markers to visible range
  marker1_ = viewXMin_ + (viewXMax_ - viewXMin_) * 0.3;
  marker2_ = viewXMin_ + (viewXMax_ - viewXMin_) * 0.7;
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

double WaveformViewWidget::fromScreenX(double sx) const {
  const double span = viewXMax_ - viewXMin_;
  const double pw = width() - kMarginL - kMarginR;
  return viewXMin_ + (sx - kMarginL) / pw * span;
}

double WaveformViewWidget::fromScreenY(double sy) const {
  const double span = viewYMax_ - viewYMin_;
  const double ph = height() - kMarginT - kMarginB;
  return viewYMax_ - (sy - kMarginT) / ph * span;
}

int WaveformViewWidget::findTrace(const std::string& name) const {
  for (std::size_t i = 0; i < traces_.size(); ++i)
    if (traces_[i].name == name) return static_cast<int>(i);
  return -1;
}

void WaveformViewWidget::addExpressionTrace(const std::string& expr,
                                             const std::string& resultName, QColor color) {
  // Simple expression parser: V(net1)-V(net2), V(net1)+V(net2), abs(V(net1)), d(V(net1))/dt
  // Find the two operands
  auto opPos = expr.find_first_of("+-");
  if (opPos == std::string::npos) return;

  std::string op1 = expr.substr(0, opPos);
  std::string op2 = expr.substr(opPos + 1);
  char op = expr[opPos];

  // Trim V() wrappers
  auto trimV = [](std::string& s) {
    auto p1 = s.find('(');
    auto p2 = s.find(')');
    if (p1 != std::string::npos && p2 != std::string::npos)
      s = s.substr(p1 + 1, p2 - p1 - 1);
  };
  trimV(op1);
  trimV(op2);

  int idx1 = findTrace(op1);
  int idx2 = findTrace(op2);
  if (idx1 < 0 || idx2 < 0) return;

  const auto& t1 = traces_[idx1];
  const auto& t2 = traces_[idx2];

  // Interpolate t2 onto t1's x-axis
  std::vector<double> result(t1.x.size());
  for (std::size_t i = 0; i < t1.x.size(); ++i) {
    double v2 = 0;
    if (t2.x.size() == t1.x.size() && std::abs(t2.x[0] - t1.x[0]) < 1e-15) {
      v2 = t2.y[i]; // Same axis, direct
    } else {
      // Interpolate
      auto it = std::upper_bound(t2.x.begin(), t2.x.end(), t1.x[i]);
      int idx = static_cast<int>(it - t2.x.begin());
      if (idx > 0 && idx < static_cast<int>(t2.x.size())) {
        double frac = (t1.x[i] - t2.x[idx - 1]) / (t2.x[idx] - t2.x[idx - 1]);
        v2 = t2.y[idx - 1] + frac * (t2.y[idx] - t2.y[idx - 1]);
      }
    }
    if (op == '-') result[i] = t1.y[i] - v2;
    else result[i] = t1.y[i] + v2;
  }

  addTrace(resultName, t1.x, std::move(result), color);
}

void WaveformViewWidget::computeEyeDiagram(double period, QColor color) {
  if (traces_.empty()) return;
  const auto& t = traces_[0];

  // Accumulate all eye segments
  std::vector<double> eyeX, eyeY;
  double tStart = t.x.front();
  double tEnd = t.x.back();
  for (double start = tStart; start + period <= tEnd; start += period) {
    double end = start + period;
    for (std::size_t i = 0; i + 1 < t.x.size(); ++i) {
      if (t.x[i] >= start && t.x[i] <= end) {
        double relX = t.x[i] - start;
        eyeX.push_back(relX);
        eyeY.push_back(t.y[i]);
      }
      if (t.x[i] < end && t.x[i + 1] > end) {
        double frac = (end - t.x[i]) / (t.x[i + 1] - t.x[i]);
        eyeX.push_back(period);
        eyeY.push_back(t.y[i] + frac * (t.y[i + 1] - t.y[i]));
      }
    }
  }

  if (!eyeX.empty()) {
    addTrace(t.name + " (Eye)", std::move(eyeX), std::move(eyeY), color);
    setAxisLabels("Time / period", "Voltage (V)");
  }
}

void WaveformViewWidget::computeFFT(QColor color) {
  if (traces_.empty()) return;
  const auto& t = traces_[0];
  if (t.x.size() < 4) return;

  // Simple DFT for frequency domain
  const double dt = (t.x.back() - t.x.front()) / (t.x.size() - 1);
  const double fs = 1.0 / dt;
  const int n = static_cast<int>(t.x.size());
  std::vector<double> freq, mag;

  // Only compute up to Nyquist
  const int nFreq = n / 2;
  for (int k = 1; k < nFreq; ++k) {
    double re = 0, im = 0;
    for (int i = 0; i < n; ++i) {
      double angle = 2.0 * M_PI * k * i / n;
      re += t.y[i] * std::cos(angle);
      im -= t.y[i] * std::sin(angle);
    }
    freq.push_back(k * fs / n);
    mag.push_back(2.0 * std::sqrt(re * re + im * im) / n);
  }

  addTrace(t.name + " (FFT)", std::move(freq), std::move(mag), color);
  setAxisLabels("Frequency (Hz)", "Magnitude");
}

// ─── Paint methods ───────────────────────────────────────────────────────────

void WaveformViewWidget::paintAxes(QPainter& p) const {
  const int pw = width() - kMarginL - kMarginR;
  const int ph = height() - kMarginT - kMarginB;

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
    if (!trace.visible || trace.x.size() < 2) continue;
    p.setPen(QPen(trace.color, 1.5));
    QPolygonF poly;
    poly.reserve(static_cast<int>(trace.x.size()));
    for (std::size_t i = 0; i < trace.x.size(); ++i)
      poly << QPointF{toScreenX(trace.x[i]), toScreenY(trace.y[i])};
    p.drawPolyline(poly);
  }
  p.setClipping(false);
}

void WaveformViewWidget::paintLegend(QPainter& p) const {
  if (traces_.empty()) return;
  p.setFont(QFont("monospace", 8));
  int x = kMarginL + 8;
  int y = kMarginT + 8;
  int maxW = 0;
  // Compute max width
  for (const auto& trace : traces_) {
    int w = p.fontMetrics().horizontalAdvance(QString::fromStdString(trace.name));
    if (w > maxW) maxW = w;
  }

  // Background box
  int boxW = maxW + 40;
  int boxH = static_cast<int>(traces_.size()) * 18 + 8;
  p.fillRect(QRectF{(double)(x - 4), (double)(y - 4), (double)boxW, (double)boxH}, QColor("#0e1318e0"));

  for (std::size_t i = 0; i < traces_.size(); ++i) {
    const auto& trace = traces_[i];
    // Draw visibility indicator (filled = visible, outline = hidden)
    if (trace.visible) {
      p.setBrush(trace.color);
      p.setPen(Qt::NoPen);
      p.drawRect(x, y + 2, 12, 8);
    } else {
      p.setPen(trace.color);
      p.setBrush(Qt::NoBrush);
      p.drawRect(x, y + 2, 12, 8);
    }
    p.setPen(trace.color);
    p.drawLine(x + 14, y + 6, x + 24, y + 6);
    p.setPen(QColor("#c8d0d8"));
    p.drawText(x + 28, y + 10, QString::fromStdString(trace.name));
    y += 18;
  }
}

void WaveformViewWidget::paintMeasurements(QPainter& p) const {
  if (traces_.empty()) return;

  // Draw marker lines
  p.setPen(QPen(QColor("#00ff88"), 1, Qt::DashLine));
  const double sx1 = toScreenX(marker1_);
  const double sx2 = toScreenX(marker2_);
  p.drawLine(QPointF{sx1, (double)kMarginT}, QPointF{sx1, (double)(height() - kMarginB)});
  p.drawLine(QPointF{sx2, (double)kMarginT}, QPointF{sx2, (double)(height() - kMarginB)});

  // Compute measurements
  const double dt = std::abs(marker2_ - marker1_);
  const double freq = (dt > 1e-15) ? 1.0 / dt : 0.0;

  // Find Y values at markers for the first trace
  auto interpY = [&](const std::vector<double>& x, const std::vector<double>& y, double xv) {
    if (x.size() < 2) return 0.0;
    auto it = std::lower_bound(x.begin(), x.end(), xv);
    int idx = static_cast<int>(it - x.begin());
    if (idx <= 0) return y.front();
    if (idx >= static_cast<int>(x.size())) return y.back();
    double frac = (xv - x[idx - 1]) / (x[idx] - x[idx - 1]);
    return y[idx - 1] + frac * (y[idx] - y[idx - 1]);
  };

  // Display measurements
  p.setPen(QColor("#00ff88"));
  p.setFont(QFont("monospace", 9));
  QStringList lines;
  lines << QString("M1: %1 s").arg(marker1_, 0, 'g', 4);
  lines << QString("M2: %1 s").arg(marker2_, 0, 'g', 4);
  lines << QString("\xCE\x94t: %1 s").arg(dt, 0, 'g', 4);
  if (freq > 0) lines << QString("Freq: %1 Hz").arg(freq, 0, 'g', 4);

  int lx = kMarginL + 8;
  int ly = kMarginT + static_cast<int>(traces_.size()) * 18 + 24;
  for (const auto& line : lines) {
    p.drawText(lx, ly, line);
    ly += 16;
  }

  // Rise/Fall time on first trace
  if (!traces_.empty()) {
    const auto& t = traces_[0];
    double vLow  = interpY(t.x, t.y, marker1_);
    double vHigh = interpY(t.x, t.y, marker2_);
    double v10 = vLow + (vHigh - vLow) * 0.1;
    double v90 = vLow + (vHigh - vLow) * 0.9;
    // Find rise time (10%→90%)
    auto findCross = [&](double target, double start, double end) {
      for (std::size_t i = 0; i + 1 < t.x.size(); ++i) {
        if (t.x[i] >= start && t.x[i] <= end) {
          if ((t.y[i] - target) * (t.y[i + 1] - target) <= 0) {
            double frac = (target - t.y[i]) / (t.y[i + 1] - t.y[i]);
            return t.x[i] + frac * (t.x[i + 1] - t.x[i]);
          }
        }
      }
      return 0.0;
    };
    double t10 = findCross(v10, marker1_, marker2_);
    double t90 = findCross(v90, marker1_, marker2_);
    double rise = (t10 > 0 && t90 > 0) ? std::abs(t90 - t10) : 0;

    if (rise > 0) {
      p.drawText(lx, ly, QString("Rise: %1 s").arg(rise, 0, 'g', 3));
      ly += 16;
    }
    // Fall time (modelled as 90%→10% of same transition)
    double t90f = findCross(v90, marker2_, marker1_);
    double t10f = findCross(v10, marker2_, marker1_);
    double fall = (t90f > 0 && t10f > 0) ? std::abs(t10f - t90f) : 0;
    if (fall > 0) {
      p.drawText(lx, ly, QString("Fall: %1 s").arg(fall, 0, 'g', 3));
    }
  }
}

// ─── Event handlers ──────────────────────────────────────────────────────────

void WaveformViewWidget::wheelEvent(QWheelEvent* event) {
  const double factor = event->angleDelta().y() > 0 ? 0.85 : 1.15;
  const double cx = fromScreenX(event->position().x());
  const double cy = fromScreenY(event->position().y());
  viewXMin_ = cx - (cx - viewXMin_) * factor;
  viewXMax_ = cx + (viewXMax_ - cx) * factor;
  viewYMin_ = cy - (cy - viewYMin_) * factor;
  viewYMax_ = cy + (viewYMax_ - cy) * factor;
  update();
}

void WaveformViewWidget::mousePressEvent(QMouseEvent* event) {
  const double sx = event->position().x();
  const double sy = event->position().y();

  // Check if clicking on legend to toggle trace visibility
  if (sy >= kMarginT + 8 && sy <= kMarginT + 8 + static_cast<int>(traces_.size()) * 18) {
    int idx = (static_cast<int>(sy) - kMarginT - 8) / 18;
    if (idx >= 0 && idx < static_cast<int>(traces_.size())) {
      traces_[idx].visible = !traces_[idx].visible;
      update();
      return;
    }
  }

  // Check if clicking near a marker
  const double hit = 8.0;
  const double sx1 = toScreenX(marker1_);
  const double sx2 = toScreenX(marker2_);

  if (std::abs(sx - sx1) < hit && sy >= kMarginT && sy <= height() - kMarginB) {
    draggedMarker_ = 0;
    return;
  }
  if (std::abs(sx - sx2) < hit && sy >= kMarginT && sy <= height() - kMarginB) {
    draggedMarker_ = 1;
    return;
  }

  if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
    panning_ = true;
    lastMousePos_ = event->position();
  }
}

void WaveformViewWidget::mouseMoveEvent(QMouseEvent* event) {
  if (draggedMarker_ >= 0) {
    double x = fromScreenX(event->position().x());
    x = std::clamp(x, viewXMin_, viewXMax_);
    if (draggedMarker_ == 0) marker1_ = x;
    else marker2_ = x;
    update();
    return;
  }
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

void WaveformViewWidget::mouseReleaseEvent(QMouseEvent*) {
  draggedMarker_ = -1;
  panning_ = false;
}

void WaveformViewWidget::keyPressEvent(QKeyEvent* event) {
  // Expression evaluation: Ctrl+E to compute V(net1)-V(net2)
  if (event->key() == Qt::Key_E && (event->modifiers() & Qt::ControlModifier)) {
    if (traces_.size() >= 2) {
      addExpressionTrace("V(" + traces_[0].name + ")-V(" + traces_[1].name + ")",
                         traces_[0].name + "-" + traces_[1].name, QColor("#ff8844"));
    }
  }
  QWidget::keyPressEvent(event);
}

void WaveformViewWidget::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.fillRect(rect(), QColor("#0e1318"));

  if (traces_.empty()) {
    p.setPen(QColor("#505a60"));
    p.drawText(rect(), Qt::AlignCenter, "No simulation data.\nRun simulation first.\n\nCtrl+E: expression math");
    return;
  }

  paintAxes(p);
  paintTraces(p);
  paintLegend(p);
  paintMeasurements(p);
}

}  // namespace aurora::ui
