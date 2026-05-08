#include "ui/SimSetupDialog.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"
#include "sim/SimRunner.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace aurora::ui {

SimSetupDialog::SimSetupDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Simulation Setup (ADE)");
  resize(560, 600);

  auto* mainLayout = new QVBoxLayout(this);

  // Simulator path
  auto* simPathBox = new QGroupBox("Simulator", this);
  auto* spLayout   = new QHBoxLayout(simPathBox);
  simPathEdit_     = new QLineEdit("ngspice", simPathBox);
  auto* browseBtn  = new QPushButton("Browse…", simPathBox);
  connect(browseBtn, &QPushButton::clicked, this, &SimSetupDialog::onBrowseSimulator);
  spLayout->addWidget(new QLabel("Path:", simPathBox));
  spLayout->addWidget(simPathEdit_);
  spLayout->addWidget(browseBtn);
  mainLayout->addWidget(simPathBox);

  // Analysis type
  auto* analysisBox = new QGroupBox("Analysis", this);
  auto* aLayout     = new QVBoxLayout(analysisBox);
  auto* typeRow     = new QHBoxLayout;
  typeRow->addWidget(new QLabel("Type:", analysisBox));
  analysisCombo_ = new QComboBox(analysisBox);
  analysisCombo_->addItems({"Operating Point (OP)", "DC Sweep", "AC Analysis", "Transient"});
  connect(analysisCombo_, qOverload<int>(&QComboBox::currentIndexChanged),
          this, &SimSetupDialog::onAnalysisTypeChanged);
  typeRow->addWidget(analysisCombo_);
  typeRow->addStretch();
  aLayout->addLayout(typeRow);

  paramStack_ = new QStackedWidget(analysisBox);

  // OP page (no params)
  auto* opPage = new QWidget(paramStack_);
  auto* opLay  = new QVBoxLayout(opPage);
  opLay->addWidget(new QLabel("No parameters needed for DC operating point.", opPage));
  paramStack_->addWidget(opPage);

  // DC Sweep page
  auto* dcPage  = new QWidget(paramStack_);
  auto* dcForm  = new QFormLayout(dcPage);
  dcSrc_   = new QLineEdit("V0", dcPage);
  dcStart_ = new QLineEdit("0", dcPage);
  dcStop_  = new QLineEdit("5", dcPage);
  dcStep_  = new QLineEdit("0.1", dcPage);
  dcForm->addRow("Source:", dcSrc_);
  dcForm->addRow("Start (V):", dcStart_);
  dcForm->addRow("Stop (V):",  dcStop_);
  dcForm->addRow("Step (V):",  dcStep_);
  paramStack_->addWidget(dcPage);

  // AC Analysis page
  auto* acPage  = new QWidget(paramStack_);
  auto* acForm  = new QFormLayout(acPage);
  acFStart_ = new QLineEdit("1", acPage);
  acFStop_  = new QLineEdit("1e9", acPage);
  acPts_    = new QLineEdit("10", acPage);
  acForm->addRow("Start Freq (Hz):", acFStart_);
  acForm->addRow("Stop Freq (Hz):",  acFStop_);
  acForm->addRow("Points/decade:",   acPts_);
  paramStack_->addWidget(acPage);

  // Transient page
  auto* tranPage  = new QWidget(paramStack_);
  auto* tranForm  = new QFormLayout(tranPage);
  tranStop_ = new QLineEdit("100n", tranPage);
  tranStep_ = new QLineEdit("1n", tranPage);
  tranForm->addRow("Stop time:", tranStop_);
  tranForm->addRow("Time step:", tranStep_);
  paramStack_->addWidget(tranPage);

  aLayout->addWidget(paramStack_);
  mainLayout->addWidget(analysisBox);

  // Output
  auto* outBox = new QGroupBox("Output", this);
  auto* outLay = new QVBoxLayout(outBox);
  outputPane_  = new QPlainTextEdit(outBox);
  outputPane_->setReadOnly(true);
  outputPane_->setMaximumBlockCount(1000);
  outputPane_->setMinimumHeight(150);
  QFont mono("Courier New", 9);
  outputPane_->setFont(mono);
  outLay->addWidget(outputPane_);
  mainLayout->addWidget(outBox);

  // Buttons
  runBtn_ = new QPushButton("Run Simulation", this);
  runBtn_->setDefault(true);
  connect(runBtn_, &QPushButton::clicked, this, &SimSetupDialog::onRun);
  auto* closeBtn = new QPushButton("Close", this);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  auto* btnRow = new QHBoxLayout;
  btnRow->addStretch();
  btnRow->addWidget(runBtn_);
  btnRow->addWidget(closeBtn);
  mainLayout->addLayout(btnRow);
}

void SimSetupDialog::onAnalysisTypeChanged(int index) {
  paramStack_->setCurrentIndex(index);
}

void SimSetupDialog::onBrowseSimulator() {
  const auto path = QFileDialog::getOpenFileName(this, "Select Simulator Executable");
  if (!path.isEmpty()) simPathEdit_->setText(path);
}

QString SimSetupDialog::buildExtraCommands() const {
  const int idx = analysisCombo_->currentIndex();
  switch (idx) {
    case 0: return ".op\n.print op all\n";
    case 1: return QString(".dc %1 %2 %3 %4\n.print dc all\n")
                .arg(dcSrc_->text()).arg(dcStart_->text()).arg(dcStop_->text()).arg(dcStep_->text());
    case 2: return QString(".ac dec %1 %2 %3\n.print ac all\n")
                .arg(acPts_->text()).arg(acFStart_->text()).arg(acFStop_->text());
    case 3: return QString(".tran %1 %2\n.print tran all\n")
                .arg(tranStep_->text()).arg(tranStop_->text());
    default: return {};
  }
}

void SimSetupDialog::onRun() {
  if (!runner_ || !lib_ || !cell_) {
    outputPane_->appendPlainText("Error: no design loaded.");
    return;
  }
  const auto* schView = cell_->findView(db::DbViewType::Schematic);
  if (!schView) {
    outputPane_->appendPlainText("Error: no schematic view for this cell.");
    return;
  }

  runner_->setSimulatorPath(std::filesystem::path(simPathEdit_->text().toStdString()));

  outputPane_->appendPlainText("Writing netlist…");
  if (!runner_->writeSpiceNetlist(*lib_, *cell_, *schView)) {
    outputPane_->appendPlainText("Error writing netlist: " +
                                  QString::fromStdString(runner_->lastError()));
    return;
  }

  outputPane_->appendPlainText("Running simulation…");
  runBtn_->setEnabled(false);
  lastResult_ = runner_->run(buildExtraCommands().toStdString());
  runBtn_->setEnabled(true);

  if (!lastResult_.success) {
    outputPane_->appendPlainText("Simulation FAILED: " +
                                  QString::fromStdString(lastResult_.errorMessage));
  } else {
    outputPane_->appendPlainText("Simulation completed.");
    if (!lastResult_.dcOperatingPoint.empty()) {
      outputPane_->appendPlainText("DC operating point:");
      for (const auto& [k, v] : lastResult_.dcOperatingPoint)
        outputPane_->appendPlainText(
            QString("  %1 = %2").arg(QString::fromStdString(k)).arg(v));
    }
  }

  outputPane_->appendPlainText(QString::fromStdString(lastResult_.rawOutput));
  emit simulationFinished(lastResult_);
}

}  // namespace aurora::ui
