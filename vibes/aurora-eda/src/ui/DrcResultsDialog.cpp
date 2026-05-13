#include "ui/DrcResultsDialog.h"
#include "ui/LayoutViewWidget.h"

#include "drc_lvs/DrcEngine.h"
#include "drc_lvs/LvsChecker.h"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace aurora::ui {

DrcResultsDialog::DrcResultsDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("DRC / LVS Results");
  resize(700, 500);

  auto* mainLay = new QVBoxLayout(this);
  tabs_ = new QTabWidget(this);

  // DRC tab
  auto* drcWidget = new QWidget(tabs_);
  auto* drcLay    = new QVBoxLayout(drcWidget);
  drcSummary_ = new QLabel("Press 'Run DRC' to check layout.", drcWidget);
  drcTable_   = new QTableWidget(0, 4, drcWidget);
  drcTable_->setHorizontalHeaderLabels({"Type", "Layer", "Location", "Message"});
  drcTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  drcTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
  drcTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  connect(drcTable_, &QTableWidget::cellDoubleClicked, this, &DrcResultsDialog::onDrcRowDoubleClicked);
  auto* runDrcBtn = new QPushButton("Run DRC", drcWidget);
  connect(runDrcBtn, &QPushButton::clicked, this, &DrcResultsDialog::onRunDrc);
  drcLay->addWidget(drcSummary_);
  drcLay->addWidget(drcTable_);
  auto* drcBtnRow = new QHBoxLayout;
  drcBtnRow->addStretch();
  drcBtnRow->addWidget(runDrcBtn);
  drcLay->addLayout(drcBtnRow);
  tabs_->addTab(drcWidget, "DRC");

  // LVS tab
  auto* lvsWidget = new QWidget(tabs_);
  auto* lvsLay    = new QVBoxLayout(lvsWidget);
  lvsSummary_ = new QLabel("Press 'Run LVS' to compare schematic vs layout.", lvsWidget);
  lvsTable_   = new QTableWidget(0, 1, lvsWidget);
  lvsTable_->setHorizontalHeaderLabels({"LVS Message"});
  lvsTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  lvsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  auto* runLvsBtn = new QPushButton("Run LVS", lvsWidget);
  connect(runLvsBtn, &QPushButton::clicked, this, &DrcResultsDialog::onRunLvs);
  lvsLay->addWidget(lvsSummary_);
  lvsLay->addWidget(lvsTable_);
  auto* lvsBtnRow = new QHBoxLayout;
  lvsBtnRow->addStretch();
  lvsBtnRow->addWidget(runLvsBtn);
  lvsLay->addLayout(lvsBtnRow);
  tabs_->addTab(lvsWidget, "LVS");

  mainLay->addWidget(tabs_);

  auto* closeBtn = new QPushButton("Close", this);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  auto* closeBtnRow = new QHBoxLayout;
  closeBtnRow->addStretch();
  closeBtnRow->addWidget(closeBtn);
  mainLay->addLayout(closeBtnRow);
}

void DrcResultsDialog::setLayoutView(const db::DbView* view, const db::DbCellLib* lib,
                                      const tech::TechDatabase* tech) {
  layView_ = view;
  lib_     = lib;
  tech_    = tech;
}

void DrcResultsDialog::setSchematicView(const db::DbView* schView, const db::DbView* layView,
                                         const db::DbCellLib* lib) {
  schView_ = schView;
  layView_ = layView;
  lib_     = lib;
}

void DrcResultsDialog::onRunDrc() {
  if (!layView_ || !tech_) {
    drcSummary_->setText("No layout view or tech database set.");
    return;
  }
  drc_lvs::DrcEngine engine(*tech_);
  drcViolations_ = engine.run(*layView_, *lib_);
  drcTable_->setRowCount((int)drcViolations_.size());

  // Push violation locations as DRC markers on the layout widget
  if (layWidget_) {
    std::vector<geom::GeomBox> markers;
    for (const auto& v : drcViolations_) {
      if (v.location.width() > 0 && v.location.height() > 0) {
        markers.push_back(v.location);
      }
    }
    layWidget_->setDrcMarkers(markers);
  }

  for (int i = 0; i < (int)drcViolations_.size(); ++i) {
    const auto& v = drcViolations_[i];
    const auto typeStr = [&]() -> QString {
      switch (v.type) {
        case drc_lvs::DrcViolationType::MinWidth:    return "MinWidth";
        case drc_lvs::DrcViolationType::MinSpacing:  return "MinSpacing";
        case drc_lvs::DrcViolationType::Enclosure:   return "Enclosure";
        case drc_lvs::DrcViolationType::NonManhattan:return "NonManhattan";
        default: return "Unknown";
      }
    }();
    drcTable_->setItem(i, 0, new QTableWidgetItem(typeStr));
    drcTable_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(v.layerName)));
    drcTable_->setItem(i, 2, new QTableWidgetItem(
        QString("(%1,%2)").arg(v.location.left()).arg(v.location.bottom())));
    drcTable_->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(v.message)));
  }

  if (drcViolations_.empty()) {
    drcSummary_->setText("DRC CLEAN — no violations.");
    drcSummary_->setStyleSheet("color: #40c040;");
  } else {
    drcSummary_->setText(QString("DRC found %1 violation(s).").arg(drcViolations_.size()));
    drcSummary_->setStyleSheet("color: #e05050;");
  }
}

void DrcResultsDialog::onRunLvs() {
  if (!schView_ || !layView_) {
    lvsSummary_->setText("Schematic or layout view not set.");
    return;
  }
  drc_lvs::LvsChecker checker;
  const auto result = checker.compare(*schView_, *layView_, *lib_);
  lvsTable_->setRowCount((int)result.errors.size());
  for (int i = 0; i < (int)result.errors.size(); ++i)
    lvsTable_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(result.errors[i])));

  if (result.matched) {
    lvsSummary_->setText("LVS PASSED — schematic matches layout.");
    lvsSummary_->setStyleSheet("color: #40c040;");
  } else {
    lvsSummary_->setText(QString("LVS FAILED — %1 error(s).").arg(result.errors.size()));
    lvsSummary_->setStyleSheet("color: #e05050;");
  }
}

void DrcResultsDialog::onDrcRowDoubleClicked(int row, int) {
  if (row < 0 || row >= (int)drcViolations_.size()) return;
  if (!layWidget_) return;
  const auto& loc = drcViolations_[row].location;
  const double cx = loc.left() / 1000.0;
  const double cy = loc.bottom() / 1000.0;
  const double sz = std::max(loc.width(), loc.height()) / 1000.0 + 2.0;
  layWidget_->zoomToBox(cx - sz, cy - sz, sz * 2, sz * 2);
}

}  // namespace aurora::ui
