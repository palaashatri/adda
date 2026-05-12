#include "ui/SimSetupDialog.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"
#include "sim/SimRunner.h"

#include <algorithm>
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
  resize(560, 700);

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
  analysisCombo_->addItems({"Operating Point (OP)", "DC Sweep", "AC Analysis",
                             "Transient", "Noise Analysis", "Distortion", "Pole-Zero"});
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

  // Noise page
  auto* noisePage = new QWidget(paramStack_);
  auto* noiseForm = new QFormLayout(noisePage);
  noiseOut_ = new QLineEdit("VOUT", noisePage);
  noiseIn_  = new QLineEdit("VIN", noisePage);
  noiseForm->addRow("Output node:", noiseOut_);
  noiseForm->addRow("Input source:", noiseIn_);
  paramStack_->addWidget(noisePage);

  // Distortion page
  auto* distoPage = new QWidget(paramStack_);
  auto* distoForm = new QFormLayout(distoPage);
  distoF1_ = new QLineEdit("1e6", distoPage);
  distoF2_ = new QLineEdit("2e6", distoPage);
  distoForm->addRow("F1 (Hz):", distoF1_);
  distoForm->addRow("F2 (Hz):", distoF2_);
  paramStack_->addWidget(distoPage);

  // Pole-Zero page
  auto* pzPage = new QWidget(paramStack_);
  auto* pzForm = new QFormLayout(pzPage);
  pzIn_ = new QLineEdit("VIN", pzPage);
  pzOut_ = new QLineEdit("VOUT", pzPage);
  pzForm->addRow("Input source:", pzIn_);
  pzForm->addRow("Output node:", pzOut_);
  paramStack_->addWidget(pzPage);

  aLayout->addWidget(paramStack_);
  mainLayout->addWidget(analysisBox);

  // Parametric sweep
  auto* sweepBox = new QGroupBox("Parametric Sweep (optional)", this);
  auto* swForm   = new QFormLayout(sweepBox);
  sweepEnable_ = new QCheckBox("Enable sweep", sweepBox);
  swForm->addRow(sweepEnable_);
  sweepParam_ = new QLineEdit("VDD", sweepBox);
  swForm->addRow("Parameter:", sweepParam_);
  sweepStart_ = new QLineEdit("0", sweepBox);
  swForm->addRow("Start:", sweepStart_);
  sweepStop_ = new QLineEdit("5", sweepBox);
  swForm->addRow("Stop:", sweepStop_);
  sweepSteps_ = new QLineEdit("10", sweepBox);
  swForm->addRow("Steps:", sweepSteps_);
  mainLayout->addWidget(sweepBox);

  // Corner simulation
  auto* cornerBox = new QGroupBox("Corner Simulation (optional)", this);
  auto* cornerForm = new QFormLayout(cornerBox);
  cornerEnable_ = new QCheckBox("Enable corners", cornerBox);
  cornerForm->addRow(cornerEnable_);
  cornerTemps_ = new QLineEdit("-40, 27, 125", cornerBox);
  cornerForm->addRow("Temperatures (°C):", cornerTemps_);
  cornerVdd_ = new QLineEdit("1.62, 1.8, 1.98", cornerBox);
  cornerForm->addRow("VDD values (V):", cornerVdd_);
  mainLayout->addWidget(cornerBox);

  // Monte Carlo
  auto* mcBox = new QGroupBox("Monte Carlo (optional)", this);
  auto* mcForm = new QFormLayout(mcBox);
  mcEnable_ = new QCheckBox("Enable Monte Carlo", mcBox);
  mcForm->addRow(mcEnable_);
  mcDistCombo_ = new QComboBox(mcBox);
  mcDistCombo_->addItems({"Gaussian", "Uniform"});
  mcForm->addRow("Distribution:", mcDistCombo_);
  mcParam1_ = new QLineEdit("1.8", mcBox);
  mcForm->addRow("Mean (Gaussian) / Min (Uniform):", mcParam1_);
  mcParam2_ = new QLineEdit("0.1", mcBox);
  mcForm->addRow("Sigma (Gaussian) / Max (Uniform):", mcParam2_);
  mcRuns_ = new QLineEdit("20", mcBox);
  mcForm->addRow("Runs:", mcRuns_);
  mainLayout->addWidget(mcBox);

  // Output
  auto* outBox = new QGroupBox("Output", this);
  auto* outLay = new QVBoxLayout(outBox);
  outputPane_  = new QPlainTextEdit(outBox);
  outputPane_->setReadOnly(true);
  outputPane_->setMaximumBlockCount(1000);
  outputPane_->setMinimumHeight(120);
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
    case 4:
      return QString(".noise v(%1) %2 dec 10 1 1e9\n.print noise all\n")
          .arg(noiseOut_->text()).arg(noiseIn_->text());
    case 5:
      return QString(".disto %1 %2\n.print disto all\n")
          .arg(distoF1_->text()).arg(distoF2_->text());
    case 6:
      return QString(".pz v(%1) %2\n.print pz all\n")
          .arg(pzOut_->text()).arg(pzIn_->text());
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

  if (cornerEnable_ && cornerEnable_->isChecked()) {
    // Corner simulation
    auto parseList = [](const QString& s) {
      std::vector<double> vals;
      for (const auto& part : s.split(',')) {
        bool ok = false;
        double v = part.trimmed().toDouble(&ok);
        if (ok) vals.push_back(v);
      }
      return vals;
    };
    auto temps = parseList(cornerTemps_->text());
    auto vdds  = parseList(cornerVdd_->text());
    if (temps.empty() || vdds.empty()) {
      outputPane_->appendPlainText("Error: invalid corner values");
      return;
    }
    outputPane_->appendPlainText(QString("Running %1 x %2 = %3 corner simulations...")
        .arg(temps.size()).arg(vdds.size()).arg(temps.size() * vdds.size()));
    runBtn_->setEnabled(false);
    auto results = runner_->runCorners(temps, vdds, buildExtraCommands().toStdString());
    runBtn_->setEnabled(true);
    outputPane_->appendPlainText(QString("Corners completed: %1 / %2 succeeded")
        .arg(std::count_if(results.begin(), results.end(),
                           [](const sim::SimResult& r) { return r.success; }))
        .arg(results.size()));
    if (!results.empty()) {
      lastResult_ = results.back();
      for (std::size_t ri = 0; ri < results.size(); ++ri) {
        for (auto& wv : results[ri].waveforms) {
          wv.name = "C" + std::to_string(ri) + " " + wv.name;
        }
        if (!results[ri].waveforms.empty())
          lastResult_.waveforms.insert(lastResult_.waveforms.end(),
              results[ri].waveforms.begin(), results[ri].waveforms.end());
      }
    }
  } else if (mcEnable_->isChecked()) {
    // Monte Carlo
    sim::MonteCarloParam mc;
    mc.name  = sweepParam_->text().toStdString();
    mc.param1 = mcParam1_->text().toDouble();
    mc.param2 = mcParam2_->text().toDouble();
    mc.runs  = mcRuns_->text().toInt();
    mc.dist  = (mcDistCombo_->currentIndex() == 0)
                 ? sim::MonteCarloParam::Gaussian : sim::MonteCarloParam::Uniform;

    const auto extraCmds = buildExtraCommands().toStdString();
    std::string cmdWithParam = ".param " + mc.name + " = $PARAM\n" + extraCmds;

    outputPane_->appendPlainText(QString("Monte Carlo: %1 runs, %2 distribution")
        .arg(mc.runs).arg(mcDistCombo_->currentText()));
    runBtn_->setEnabled(false);
    auto results = runner_->runMonteCarlo(mc, cmdWithParam);
    runBtn_->setEnabled(true);

    outputPane_->appendPlainText(
        QString("MC completed: %1 / %2 succeeded")
            .arg(std::count_if(results.begin(), results.end(),
                               [](const sim::SimResult& r) { return r.success; }))
            .arg(results.size()));
    if (!results.empty()) {
      lastResult_ = results.back();
      for (std::size_t ri = 0; ri < results.size(); ++ri) {
        for (auto& wv : results[ri].waveforms) {
          wv.name = "MC" + std::to_string(ri) + " " + wv.name;
        }
        if (!results[ri].waveforms.empty()) {
          lastResult_.waveforms.insert(lastResult_.waveforms.end(),
              results[ri].waveforms.begin(), results[ri].waveforms.end());
        }
      }
    }
  } else if (sweepEnable_->isChecked()) {
    sim::SweepParam param;
    param.name    = sweepParam_->text().toStdString();
    param.start   = sweepStart_->text().toDouble();
    param.stop    = sweepStop_->text().toDouble();
    param.steps   = sweepSteps_->text().toInt();
    param.logScale = false;

    const auto extraCmds = buildExtraCommands().toStdString();
    std::string cmdWithParam = ".param " + param.name + " = $PARAM\n" + extraCmds;

    outputPane_->appendPlainText(QString("Sweeping %1 from %2 to %3 (%4 steps)…")
        .arg(sweepParam_->text()).arg(sweepStart_->text())
        .arg(sweepStop_->text()).arg(sweepSteps_->text()));
    runBtn_->setEnabled(false);
    auto results = runner_->runSweep(param, cmdWithParam);
    runBtn_->setEnabled(true);

    outputPane_->appendPlainText(
        QString("Sweep completed: %1 / %2 succeeded")
            .arg(std::count_if(results.begin(), results.end(),
                               [](const sim::SimResult& r) { return r.success; }))
            .arg(results.size()));
    if (!results.empty()) {
      lastResult_ = results.back();
      for (std::size_t ri = 0; ri < results.size(); ++ri) {
        for (auto& wv : results[ri].waveforms) {
          wv.name = param.name + "=" + std::to_string(
              param.start + (param.stop - param.start) * ri / param.steps) + " " + wv.name;
        }
        if (!results[ri].waveforms.empty()) {
          lastResult_.waveforms.insert(lastResult_.waveforms.end(),
              results[ri].waveforms.begin(), results[ri].waveforms.end());
        }
      }
    }
  } else {
    outputPane_->appendPlainText("Running simulation…");
    runBtn_->setEnabled(false);
    lastResult_ = runner_->run(buildExtraCommands().toStdString());
    runBtn_->setEnabled(true);
  }

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
    outputPane_->appendPlainText(
        QString("Waveforms: %1 traces").arg(lastResult_.waveforms.size()));
  }

  outputPane_->appendPlainText(QString::fromStdString(lastResult_.rawOutput));
  emit simulationFinished(lastResult_);
}

}  // namespace aurora::ui
