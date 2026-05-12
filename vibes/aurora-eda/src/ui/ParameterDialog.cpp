#include "ui/ParameterDialog.h"

#include "db/DbInstance.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace aurora::ui {

ParameterDialog::ParameterDialog(db::DbInstance& inst, QWidget* parent)
    : QDialog(parent), inst_(inst) {
  setWindowTitle(QString("Parameters: %1").arg(QString::fromStdString(inst.name())));
  resize(400, 300);

  auto* lay = new QVBoxLayout(this);
  lay->addWidget(new QLabel(QString("Instance: %1").arg(QString::fromStdString(inst.name()))));

  table_ = new QTableWidget(0, 2, this);
  table_->setHorizontalHeaderLabels({"Parameter", "Value"});
  table_->horizontalHeader()->setStretchLastSection(true);

  // Populate from instance
  int row = 0;
  for (const auto& [k, v] : inst.parameters()) {
    table_->insertRow(row);
    table_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(k)));
    table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(v)));
    ++row;
  }
  lay->addWidget(table_);

  auto* btnRow = new QHBoxLayout;
  auto* addBtn = new QPushButton("Add", this);
  connect(addBtn, &QPushButton::clicked, this, &ParameterDialog::onAdd);
  auto* remBtn = new QPushButton("Remove", this);
  connect(remBtn, &QPushButton::clicked, this, &ParameterDialog::onRemove);
  btnRow->addWidget(addBtn);
  btnRow->addWidget(remBtn);
  btnRow->addStretch();

  auto* okBtn = new QPushButton("OK", this);
  connect(okBtn, &QPushButton::clicked, this, [this]() {
    // Save parameters
    inst_.clearParameters();
    for (int r = 0; r < table_->rowCount(); ++r) {
      auto* kItem = table_->item(r, 0);
      auto* vItem = table_->item(r, 1);
      if (kItem && vItem && !kItem->text().isEmpty())
        inst_.setParameter(kItem->text().toStdString(), vItem->text().toStdString());
    }
    accept();
  });
  btnRow->addWidget(okBtn);
  auto* cancelBtn = new QPushButton("Cancel", this);
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  btnRow->addWidget(cancelBtn);
  lay->addLayout(btnRow);
}

void ParameterDialog::onAdd() {
  int row = table_->rowCount();
  table_->insertRow(row);
  table_->setItem(row, 0, new QTableWidgetItem("param"));
  table_->setItem(row, 1, new QTableWidgetItem("value"));
}

void ParameterDialog::onRemove() {
  int row = table_->currentRow();
  if (row >= 0 && row < table_->rowCount())
    table_->removeRow(row);
}

}  // namespace aurora::ui
