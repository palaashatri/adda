#pragma once

#include "sim/SimResult.h"

#include <QCheckBox>
#include <QDialog>
#include <QString>

#include <vector>

class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QGroupBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;

namespace aurora {
namespace db { class DbCellLib; class DbCell; }
namespace sim { class SimRunner; struct SweepParam; }
}  // namespace aurora

namespace aurora::ui {

class SimSetupDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SimSetupDialog(QWidget* parent = nullptr);

  void setSimRunner(sim::SimRunner* runner) { runner_ = runner; }
  void setCell(const db::DbCellLib* lib, const db::DbCell* cell) { lib_ = lib; cell_ = cell; }

  [[nodiscard]] const sim::SimResult& lastResult() const { return lastResult_; }

 signals:
  void simulationFinished(const sim::SimResult& result);

 private slots:
  void onAnalysisTypeChanged(int index);
  void onRun();
  void onBrowseSimulator();

 private:
  QComboBox*       analysisCombo_{nullptr};
  QStackedWidget*  paramStack_{nullptr};
  QLineEdit*       simPathEdit_{nullptr};
  QPlainTextEdit*  outputPane_{nullptr};
  QPushButton*     runBtn_{nullptr};

  // Per-analysis param widgets
  QLineEdit* dcSrc_{nullptr};
  QLineEdit* dcStart_{nullptr};
  QLineEdit* dcStop_{nullptr};
  QLineEdit* dcStep_{nullptr};

  QLineEdit* acFStart_{nullptr};
  QLineEdit* acFStop_{nullptr};
  QLineEdit* acPts_{nullptr};

  QLineEdit* tranStop_{nullptr};
  QLineEdit* tranStep_{nullptr};

  // Sweep param widgets
  QCheckBox* sweepEnable_{nullptr};
  QLineEdit* sweepParam_{nullptr};
  QLineEdit* sweepStart_{nullptr};
  QLineEdit* sweepStop_{nullptr};
  QLineEdit* sweepSteps_{nullptr};

  sim::SimRunner*      runner_{nullptr};
  const db::DbCellLib* lib_{nullptr};
  const db::DbCell*    cell_{nullptr};
  sim::SimResult       lastResult_;

  [[nodiscard]] QString buildExtraCommands() const;
};

}  // namespace aurora::ui
