#pragma once

#include <QWidget>
#include <QString>
#include <QColor>

#include <map>
#include <string>
#include <vector>

namespace aurora::ui {

struct WaveformTrace {
  std::string name;
  std::vector<double> x;
  std::vector<double> y;
  QColor color;
  bool visible{true};
};

class WaveformViewWidget : public QWidget {
  Q_OBJECT

 public:
  explicit WaveformViewWidget(QWidget* parent = nullptr);

  void addTrace(std::string name, std::vector<double> x, std::vector<double> y,
                QColor color = QColor("#00ccff"));
  void clearTraces();
  void setAxisLabels(const QString& xLabel, const QString& yLabel);
  void fitView();

  // Trace visibility toggle (D13)
  [[nodiscard]] int traceCount() const { return static_cast<int>(traces_.size()); }
  [[nodiscard]] std::string traceName(int i) const { return traces_[i].name; }
  [[nodiscard]] bool traceVisible(int i) const { return traces_[i].visible; }
  void setTraceVisible(int i, bool v) { traces_[i].visible = v; update(); }

  // Eye diagram (D10): overlay time segments of length `period`
  void computeEyeDiagram(double period, QColor color = QColor("#44ff88"));

  // FFT (D9): compute FFT of the first trace and add as new trace
  void computeFFT(QColor color = QColor("#ff44cc"));

  // Expression math (D8): compute new trace from existing traces
  // Supported: V(name1)-V(name2), V(name1)+V(name2), abs(V(name1)), d(V(name1))/dt
  void addExpressionTrace(const std::string& expr, const std::string& resultName,
                          QColor color = QColor("#ffcc00"));

 protected:
  void paintEvent(QPaintEvent*) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  void computeBounds();
  [[nodiscard]] double toScreenX(double x) const;
  [[nodiscard]] double toScreenY(double y) const;
  [[nodiscard]] double fromScreenX(double sx) const;
  [[nodiscard]] double fromScreenY(double sy) const;
  void paintAxes(QPainter& p) const;
  void paintTraces(QPainter& p) const;
  void paintLegend(QPainter& p) const;
  void paintMeasurements(QPainter& p) const;

  // Find trace by name, return index or -1
  [[nodiscard]] int findTrace(const std::string& name) const;

  std::vector<WaveformTrace> traces_;
  QString xLabel_{"Time (s)"};
  QString yLabel_{"Voltage (V)"};

  double xMin_{0.0}, xMax_{1.0};
  double yMin_{-1.0}, yMax_{1.0};

  double viewXMin_{0.0}, viewXMax_{1.0};
  double viewYMin_{-1.0}, viewYMax_{1.0};

  QPointF lastMousePos_;
  bool panning_{false};

  // Measurement cursors (D15)
  double marker1_{0.5}, marker2_{0.6}; // X values
  int draggedMarker_{-1}; // -1=none, 0=m1, 1=m2

  static constexpr int kMarginL{60};
  static constexpr int kMarginR{20};
  static constexpr int kMarginT{20};
  static constexpr int kMarginB{60}; // extra space for expression input
};

}  // namespace aurora::ui
