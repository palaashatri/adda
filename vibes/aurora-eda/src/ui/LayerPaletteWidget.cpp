#include "ui/LayerPaletteWidget.h"

#include <QListWidget>
#include <QVBoxLayout>

namespace aurora::ui {

LayerPaletteWidget::LayerPaletteWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  auto* layers = new QListWidget(this);
  layers->addItem("metal1/drawing");
  layers->addItem("poly/drawing");
  layers->addItem("diff/drawing");
  layout->addWidget(layers);
}

}  // namespace aurora::ui
