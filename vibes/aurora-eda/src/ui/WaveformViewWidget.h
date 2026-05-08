#pragma once

#include <QWidget>
#include <QString>
#include <QColor>

#include <vector>
#include <string>

namespace aurora::ui {

struct WaveformTrace {
  std::string name;
  std::vector<double> x;
  std::vector<double> y;
  QColor color;
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

 protected:
  void paintEvent(QPaintEvent*) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  void computeBounds();
  [[nodiscard]] double toScreenX(double x) const;
  [[nodiscard]] double toScreenY(double y) const;
  void paintAxes(QPainter& p) const;
  void paintTraces(QPainter& p) const;
  void paintLegend(QPainter& p) const;

  std::vector<WaveformTrace> traces_;
  QString xLabel_{"Time (s)"};
  QString yLabel_{"Voltage (V)"};

  double xMin_{0.0}, xMax_{1.0};
  double yMin_{-1.0}, yMax_{1.0};

  double viewXMin_{0.0}, viewXMax_{1.0};
  double viewYMin_{-1.0}, viewYMax_{1.0};

  QPointF lastMousePos_;
  bool panning_{false};

  static constexpr int kMarginL{60};
  static constexpr int kMarginR{20};
  static constexpr int kMarginT{20};
  static constexpr int kMarginB{40};
};

}  // namespace aurora::ui
