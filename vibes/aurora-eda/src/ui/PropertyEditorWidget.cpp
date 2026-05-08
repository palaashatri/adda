#include "ui/PropertyEditorWidget.h"

#include <QFormLayout>
#include <QLabel>

namespace aurora::ui {

PropertyEditorWidget::PropertyEditorWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QFormLayout(this);
  layout->addRow("Selection", new QLabel("None", this));
  layout->addRow("Parameters", new QLabel("-", this));
}

}  // namespace aurora::ui
