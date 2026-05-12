#include "ui/SimSetupDialog.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbView.h"
#include "sim/SimRunner.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
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
  // Testbench management (D11)
  auto* tbBox = new QGroupBox("Testbench", this);
  auto* tbLayout = new QHBoxLayout(tbBox);
  tbCombo_ = new QComboBox(tbBox);
  tbCombo_->setEditable(true);
  tbCombo_->setMinimumWidth(200);
  tbLayout->addWidget(tbCombo_);
  auto* tbNewBtn = new QPushButton("New", tbBox);
  connect(tbNewBtn, &QPushButton::clicked, this, &SimSetupDialog::onTestbenchNew);
  tbLayout->addWidget(tbNewBtn);
  tbSaveBtn_ = new QPushButton("Save", tbBox);
  connect(tbSaveBtn_, &QPushButton::clicked, this, &SimSetupDialog::onTestbenchSave);
  tbLayout->addWidget(tbSaveBtn_);
  tbDeleteBtn_ = new QPushButton("Delete", tbBox);
  connect(tbDeleteBtn_, &QPushButton::clicked, this, &SimSetupDialog::onTestbenchDelete);
  tbLayout->addWidget(tbDeleteBtn_);
  mainLayout->addWidget(tbBox);

  auto* simPathBox = new QGroupBox("Simulator", this);
  auto* spLayout   = new QHBoxLayout(simPathBox);
  simPathEdit_     = new QLineEdit("ngspice", simPathBox);
  auto* browseBtn  = new QPushButton("Browse…", simPathBox);
  connect(browseBtn, &QPushButton::clicked, this, &SimSetupDialog::onBrowseSimulator);
  spLayout->addWidget(new QLabel("Path:", simPathBox));
  spLayout->addWidget(simPathEdit_);
  spLayout->addWidget(browseBtn);
  spLayout->addWidget(new QLabel("Type:", simPathBox));
  simTypeCombo_ = new QComboBox(simPathBox);
  simTypeCombo_->addItems({"ngspice", "Xyce"});
  connect(simTypeCombo_, &QComboBox::currentTextChanged, this, [this](const QString& t) {
    if (t == "Xyce") simPathEdit_->setText("xyce");
    else simPathEdit_->setText("ngspice");
  });
  spLayout->addWidget(simTypeCombo_);
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

  // Design optimization (D7)
  optBox_ = new QGroupBox("Design Optimization (optional)", this);
  auto* optForm = new QFormLayout(optBox_);
  optParam_ = new QLineEdit("W", optBox_);
  optForm->addRow("Parameter:", optParam_);
  optStart_ = new QLineEdit("1e-6", optBox_);
  optForm->addRow("Start:", optStart_);
  optStop_ = new QLineEdit("10e-6", optBox_);
  optForm->addRow("Stop:", optStop_);
  optSteps_ = new QLineEdit("10", optBox_);
  optForm->addRow("Steps:", optSteps_);
  optTarget_ = new QLineEdit("v(out) > 1.8", optBox_);
  optForm->addRow("Target (e.g., v(out)>1.8):", optTarget_);
  optBtn_ = new QPushButton("Optimize", optBox_);
  connect(optBtn_, &QPushButton::clicked, this, &SimSetupDialog::onOptimize);
  optForm->addRow(optBtn_);
  mainLayout->addWidget(optBox_);

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
  auto* saveBtn = new QPushButton("Save Setup", this);
  connect(saveBtn, &QPushButton::clicked, this, [this]() {
    auto path = QFileDialog::getSaveFileName(this, "Save Simulation Setup",
        QDir::homePath(), "JSON (*.json);;All Files (*)");
    if (!path.isEmpty()) { saveState(path); outputPane_->appendPlainText("Setup saved: " + path); }
  });
  auto* loadBtn = new QPushButton("Load Setup", this);
  connect(loadBtn, &QPushButton::clicked, this, [this]() {
    auto path = QFileDialog::getOpenFileName(this, "Load Simulation Setup",
        QDir::homePath(), "JSON (*.json);;All Files (*)");
    if (!path.isEmpty()) { loadState(path); outputPane_->appendPlainText("Setup loaded: " + path); }
  });
  btnRow->addWidget(saveBtn);
  btnRow->addWidget(loadBtn);
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

void SimSetupDialog::saveState(const QString& path) const {
  nlohmann::json j;
  j["simulator_path"] = simPathEdit_->text().toStdString();
  j["analysis_type"] = analysisCombo_->currentIndex();
  j["dc_src"] = dcSrc_->text().toStdString();
  j["dc_start"] = dcStart_->text().toStdString();
  j["dc_stop"] = dcStop_->text().toStdString();
  j["dc_step"] = dcStep_->text().toStdString();
  j["ac_fstart"] = acFStart_->text().toStdString();
  j["ac_fstop"] = acFStop_->text().toStdString();
  j["ac_pts"] = acPts_->text().toStdString();
  j["tran_stop"] = tranStop_->text().toStdString();
  j["tran_step"] = tranStep_->text().toStdString();
  j["sweep_enable"] = sweepEnable_->isChecked();
  j["sweep_param"] = sweepParam_->text().toStdString();
  j["sweep_start"] = sweepStart_->text().toStdString();
  j["sweep_stop"] = sweepStop_->text().toStdString();
  j["sweep_steps"] = sweepSteps_->text().toStdString();
  std::ofstream ofs(path.toStdString());
  if (ofs) ofs << j.dump(2);
}

void SimSetupDialog::loadState(const QString& path) {
  std::ifstream ifs(path.toStdString());
  if (!ifs) return;
  nlohmann::json j;
  try { ifs >> j; } catch (...) { return; }
  simPathEdit_->setText(QString::fromStdString(j.value("simulator_path", "")));
  analysisCombo_->setCurrentIndex(j.value("analysis_type", 0));
  if (j.contains("dc_src")) dcSrc_->setText(QString::fromStdString(j["dc_src"]));
  if (j.contains("dc_start")) dcStart_->setText(QString::fromStdString(j["dc_start"]));
  if (j.contains("dc_stop")) dcStop_->setText(QString::fromStdString(j["dc_stop"]));
  if (j.contains("dc_step")) dcStep_->setText(QString::fromStdString(j["dc_step"]));
  if (j.contains("ac_fstart")) acFStart_->setText(QString::fromStdString(j["ac_fstart"]));
  if (j.contains("ac_fstop")) acFStop_->setText(QString::fromStdString(j["ac_fstop"]));
  if (j.contains("ac_pts")) acPts_->setText(QString::fromStdString(j["ac_pts"]));
  if (j.contains("tran_stop")) tranStop_->setText(QString::fromStdString(j["tran_stop"]));
  if (j.contains("tran_step")) tranStep_->setText(QString::fromStdString(j["tran_step"]));
  if (j.contains("sweep_enable")) sweepEnable_->setChecked(j["sweep_enable"]);
  if (j.contains("sweep_param")) sweepParam_->setText(QString::fromStdString(j["sweep_param"]));
  if (j.contains("sweep_start")) sweepStart_->setText(QString::fromStdString(j["sweep_start"]));
  if (j.contains("sweep_stop")) sweepStop_->setText(QString::fromStdString(j["sweep_stop"]));
  if (j.contains("sweep_steps")) sweepSteps_->setText(QString::fromStdString(j["sweep_steps"]));
}

// ─── Testbench management ────────────────────────────────────────────────────

void SimSetupDialog::refreshTestbenchList() {
  tbCombo_->clear();
  for (const auto& name : testbenches_)
    tbCombo_->addItem(name);
}

void SimSetupDialog::onTestbenchNew() {
  bool ok = false;
  const QString name = QInputDialog::getText(this, "New Testbench",
      "Name:", QLineEdit::Normal, "tb1", &ok);
  if (!ok || name.isEmpty()) return;
  testbenches_.push_back(name);
  refreshTestbenchList();
  tbCombo_->setCurrentText(name);
  // Save empty state for this testbench
  QString dir = tbDir_.isEmpty() ? QDir::homePath() : tbDir_;
  saveState(dir + "/" + name + ".json");
  outputPane_->appendPlainText("Testbench created: " + name);
}

void SimSetupDialog::onTestbenchSave() {
  if (tbDir_.isEmpty()) tbDir_ = QDir::homePath();
  const QString name = tbCombo_->currentText();
  if (name.isEmpty()) return;
  if (std::find(testbenches_.begin(), testbenches_.end(), name) == testbenches_.end()) {
    testbenches_.push_back(name);
    refreshTestbenchList();
    tbCombo_->setCurrentText(name);
  }
  saveState(tbDir_ + "/" + name + ".json");
  outputPane_->appendPlainText("Saved: " + name);
}

void SimSetupDialog::onTestbenchLoad() {
  const QString name = tbCombo_->currentText();
  if (name.isEmpty()) return;
  if (tbDir_.isEmpty()) tbDir_ = QDir::homePath();
  loadState(tbDir_ + "/" + name + ".json");
  outputPane_->appendPlainText("Loaded: " + name);
}

void SimSetupDialog::onTestbenchDelete() {
  const QString name = tbCombo_->currentText();
  if (name.isEmpty() || std::find(testbenches_.begin(), testbenches_.end(), name) == testbenches_.end()) return;
  testbenches_.erase(std::remove(testbenches_.begin(), testbenches_.end(), name),
                     testbenches_.end());
  refreshTestbenchList();
  outputPane_->appendPlainText("Deleted: " + name);
}

// ─── Design Optimization ─────────────────────────────────────────────────────

void SimSetupDialog::onOptimize() {
  if (!runner_ || !lib_ || !cell_) {
    outputPane_->appendPlainText("Error: no design loaded.");
    return;
  }
  const auto* schView = cell_->findView(db::DbViewType::Schematic);
  if (!schView) { outputPane_->appendPlainText("Error: no schematic."); return; }

  const std::string paramName = optParam_->text().toStdString();
  const double start = optStart_->text().toDouble();
  const double stop = optStop_->text().toDouble();
  const int steps = optSteps_->text().toInt();
  const std::string targetExpr = optTarget_->text().toStdString();

  outputPane_->appendPlainText(QString("Optimizing %1 from %2 to %3 (%4 steps)...")
      .arg(optParam_->text()).arg(optStart_->text()).arg(optStop_->text()).arg(steps));

  runBtn_->setEnabled(false);
  sim::SweepParam sp{paramName, start, stop, steps, false};
  const auto extraCmds = buildExtraCommands().toStdString();
  std::string cmdWithParam = ".param " + paramName + " = $PARAM\n" + extraCmds;
  auto results = runner_->runSweep(sp, cmdWithParam);
  runBtn_->setEnabled(true);

  // Find best result based on a simple rule: maximize/minimize a value
  double bestVal = 0;
  int bestIdx = 0;
  for (std::size_t i = 0; i < results.size(); ++i) {
    if (!results[i].success) continue;
    // Try to extract meaningful metric from DC operating point
    for (const auto& [k, v] : results[i].dcOperatingPoint) {
      if (k.find(targetExpr) != std::string::npos || targetExpr.find(k) != std::string::npos) {
        if (std::abs(v) > std::abs(bestVal)) { bestVal = v; bestIdx = static_cast<int>(i); }
      }
    }
  }

  double optValue = start + (stop - start) * bestIdx / steps;
  outputPane_->appendPlainText(QString("Optimization complete. Best value at %1 = %2")
      .arg(optParam_->text()).arg(optValue));

  if (bestIdx < static_cast<int>(results.size()) && results[bestIdx].success) {
    lastResult_ = results[bestIdx];
    emit simulationFinished(lastResult_);
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
