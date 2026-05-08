#include "ui/PropertyEditorWidget.h"

#include "db/DbCellLib.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"
#include "db/DbShape.h"
#include "db/DbView.h"

#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace aurora::ui {

PropertyEditorWidget::PropertyEditorWidget(QWidget* parent) : QWidget(parent) {
  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);

  auto* scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  outer->addWidget(scroll);

  content_ = new QWidget(scroll);
  form_ = new QFormLayout(content_);
  form_->setLabelAlignment(Qt::AlignRight);
  scroll->setWidget(content_);

  addRow("Selection", "None");
}

void PropertyEditorWidget::clearRows() {
  while (form_->rowCount() > 0) form_->removeRow(0);
}

void PropertyEditorWidget::addRow(const QString& label, const QString& value) {
  auto* lbl = new QLabel(value, content_);
  lbl->setWordWrap(true);
  form_->addRow(label + ":", lbl);
}

void PropertyEditorWidget::clear() {
  clearRows();
  addRow("Selection", "None");
}

void PropertyEditorWidget::showInstance(const db::DbView& view, db::DbId instanceId,
                                        const db::DbCellLib* lib) {
  const auto* inst = view.findInstance(instanceId);
  if (!inst) { clear(); return; }
  clearRows();
  addRow("Type", "Instance");
  addRow("Name", QString::fromStdString(inst->name()));
  if (lib) {
    if (const auto* cell = lib->findCellById(inst->masterCellId()))
      addRow("Master Cell", QString::fromStdString(cell->name()));
  }
  addRow("dx", QString::number(inst->transform().dx));
  addRow("dy", QString::number(inst->transform().dy));
  addRow("Rotation", QString::number(inst->transform().rotationDegrees) + "°");
  addRow("Mirror X", inst->transform().mirrorX ? "Yes" : "No");
  for (const auto& [k, v] : inst->parameters())
    addRow(QString::fromStdString(k), QString::fromStdString(v));
}

void PropertyEditorWidget::showShape(const db::DbView& view, db::DbId shapeId) {
  const auto* shape = view.findShape(shapeId);
  if (!shape) { clear(); return; }
  clearRows();
  addRow("Type", "Shape");
  switch (shape->kind()) {
    case db::DbShapeKind::Rect: {
      addRow("Kind", "Rectangle");
      const auto& b = static_cast<const db::DbRect*>(shape)->box();
      addRow("Left",   QString::number(b.left()));
      addRow("Bottom", QString::number(b.bottom()));
      addRow("Right",  QString::number(b.right()));
      addRow("Top",    QString::number(b.top()));
      addRow("Width",  QString::number(b.width()));
      addRow("Height", QString::number(b.height()));
      break;
    }
    case db::DbShapeKind::Polygon:
      addRow("Kind", "Polygon");
      addRow("Vertices", QString::number(
          static_cast<const db::DbPolygon*>(shape)->polygon().points().size()));
      break;
    case db::DbShapeKind::Path:
      addRow("Kind", "Path");
      addRow("Width", QString::number(static_cast<const db::DbPath*>(shape)->path().width()));
      addRow("Points", QString::number(
          static_cast<const db::DbPath*>(shape)->path().points().size()));
      break;
    case db::DbShapeKind::Text:
      addRow("Kind", "Text");
      addRow("Text", QString::fromStdString(static_cast<const db::DbText*>(shape)->text()));
      break;
  }
  addRow("Layer ID", QString::number(shape->layerId()));
}

void PropertyEditorWidget::showNet(const db::DbView& view, db::DbId netId) {
  const auto* net = view.findNet(netId);
  if (!net) { clear(); return; }
  clearRows();
  addRow("Type", "Net");
  addRow("Name", QString::fromStdString(net->name()));
  addRow("Pins", QString::number(net->pinIds().size()));
}

}  // namespace aurora::ui
