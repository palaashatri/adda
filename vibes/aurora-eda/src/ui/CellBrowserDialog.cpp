#include "ui/CellBrowserDialog.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbTypes.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace aurora::ui {

CellBrowserDialog::CellBrowserDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle("Library Browser");
  resize(480, 360);

  auto* mainLay = new QHBoxLayout(this);

  auto* leftLay = new QVBoxLayout;
  leftLay->addWidget(new QLabel("Cells:", this));
  cellTree_ = new QTreeWidget(this);
  cellTree_->setHeaderLabel("Cell");
  cellTree_->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(cellTree_, &QTreeWidget::itemSelectionChanged, this,
          &CellBrowserDialog::onCellSelectionChanged);
  leftLay->addWidget(cellTree_);
  newCellBtn_ = new QPushButton("New Cell…", this);
  connect(newCellBtn_, &QPushButton::clicked, this, &CellBrowserDialog::onNewCell);
  leftLay->addWidget(newCellBtn_);
  mainLay->addLayout(leftLay);

  auto* rightLay = new QVBoxLayout;
  rightLay->addWidget(new QLabel("Views:", this));
  viewList_ = new QListWidget(this);
  connect(viewList_, &QListWidget::itemDoubleClicked, this,
          &CellBrowserDialog::onViewDoubleClicked);
  rightLay->addWidget(viewList_);

  openBtn_ = new QPushButton("Open", this);
  openBtn_->setDefault(true);
  connect(openBtn_, &QPushButton::clicked, this, &CellBrowserDialog::onViewDoubleClicked);
  auto* closeBtn = new QPushButton("Cancel", this);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
  auto* btnRow = new QHBoxLayout;
  btnRow->addStretch();
  btnRow->addWidget(openBtn_);
  btnRow->addWidget(closeBtn);
  rightLay->addLayout(btnRow);
  mainLay->addLayout(rightLay);
}

void CellBrowserDialog::setLibrary(const db::DbCellLib* lib) {
  lib_ = lib;
  cellTree_->clear();
  if (!lib_) return;
  for (const auto id : lib_->cellIds()) {
    const auto* cell = lib_->findCellById(id);
    if (!cell) continue;
    auto* item = new QTreeWidgetItem(cellTree_, {QString::fromStdString(cell->name())});
    item->setData(0, Qt::UserRole, static_cast<qlonglong>(id));
  }
}

void CellBrowserDialog::onCellSelectionChanged() {
  viewList_->clear();
  const auto* item = cellTree_->currentItem();
  if (!item || !lib_) return;
  const auto cellId = static_cast<db::DbId>(item->data(0, Qt::UserRole).toLongLong());
  const auto* cell = lib_->findCellById(cellId);
  if (!cell) return;
  selectedCellId_ = cellId;
  for (const auto vid : cell->viewIds()) {
    const auto* view = cell->findViewById(vid);
    if (!view) continue;
    auto* vitem = new QListWidgetItem(
        QString::fromStdString(std::string(db::toString(view->type()))), viewList_);
    vitem->setData(Qt::UserRole, static_cast<int>(view->type()));
  }
}

void CellBrowserDialog::onViewDoubleClicked() {
  const auto* vitem = viewList_->currentItem();
  if (!vitem) return;
  selectedViewType_ = static_cast<db::DbViewType>(vitem->data(Qt::UserRole).toInt());
  accept();
}

void CellBrowserDialog::onNewCell() {
  // Just close with a special signal — MainWindow will create the cell
  bool ok = false;
  const auto name = QInputDialog::getText(this, "New Cell", "Cell name:", QLineEdit::Normal,
                                          "untitled", &ok);
  if (ok && !name.isEmpty()) {
    // We can't create cells here without a non-const lib pointer;
    // emit the dialog accepted with an invalid ID to signal "new cell" mode
    selectedCellId_ = db::kInvalidId;
    setWindowTitle(name);  // pass name via title hack for now
    accept();
  }
}

}  // namespace aurora::ui
