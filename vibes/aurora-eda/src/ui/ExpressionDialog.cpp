#include "ui/ExpressionDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace aurora::ui {

ExpressionDialog::ExpressionDialog(const QStringList& traceNames, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle("Waveform Expression Editor");
  resize(450, 350);

  auto* lay = new QVBoxLayout(this);

  lay->addWidget(new QLabel("Available traces (click to insert):"));
  traceList_ = new QListWidget(this);
  for (const auto& name : traceNames) traceList_->addItem(name);
  lay->addWidget(traceList_);

  auto* exprRow = new QHBoxLayout;
  exprRow->addWidget(new QLabel("Expression:"));
  exprEdit_ = new QLineEdit(this);
  exprRow->addWidget(exprEdit_);
  lay->addLayout(exprRow);

  auto* opRow = new QHBoxLayout;
  auto makeOp = [&](const QString& label) {
    auto* btn = new QPushButton(label, this);
    connect(btn, &QPushButton::clicked, this, [this, label]() { onAddOp(label); });
    opRow->addWidget(btn);
  };
  makeOp(" + "); makeOp(" - "); makeOp(" * ");
  makeOp(" abs()"); makeOp(" sqrt()");
  makeOp("V("); makeOp(")");
  auto* clearBtn = new QPushButton("Clear", this);
  connect(clearBtn, &QPushButton::clicked, this, &ExpressionDialog::onClear);
  opRow->addWidget(clearBtn);
  lay->addLayout(opRow);

  auto* resRow = new QHBoxLayout;
  resRow->addWidget(new QLabel("Result name:"));
  resultEdit_ = new QLineEdit("result", this);
  resRow->addWidget(resultEdit_);
  lay->addLayout(resRow);

  auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
  lay->addWidget(btns);

  connect(traceList_, &QListWidget::itemDoubleClicked, this, [this](auto* item) {
    exprEdit_->insert(item->text());
  });
}

QString ExpressionDialog::expression() const { return exprEdit_->text(); }
QString ExpressionDialog::resultName() const { return resultEdit_->text(); }

void ExpressionDialog::onAddOp(const QString& op) { exprEdit_->insert(op); }
void ExpressionDialog::onClear() { exprEdit_->clear(); }

}  // namespace aurora::ui
