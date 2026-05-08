#include "ui/LayerPaletteWidget.h"

#include "db/DbCellLib.h"
#include "tech/TechDatabase.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QColor>
#include <QIcon>
#include <QPixmap>

namespace aurora::ui {

LayerPaletteWidget::LayerPaletteWidget(QWidget* parent) : QWidget(parent) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  
  list_ = new QListWidget(this);
  layout->addWidget(list_);

  connect(list_, &QListWidget::itemChanged, this, &LayerPaletteWidget::onItemChanged);
  connect(list_, &QListWidget::itemSelectionChanged, this, &LayerPaletteWidget::onSelectionChanged);
}

void LayerPaletteWidget::setLibrary(const db::DbCellLib* lib, const tech::TechDatabase* tech) {
  lib_ = lib;
  tech_ = tech;
  list_->clear();
  if (!lib_) return;

  for (const auto layerId : lib_->layerIds()) {
    const auto* layer = lib_->findLayer(layerId);
    if (!layer) continue;

    QString label = QString::fromStdString(layer->name());
    if (!layer->purpose().empty()) {
      label += "/" + QString::fromStdString(layer->purpose());
    }

    auto* item = new QListWidgetItem(label, list_);
    item->setData(Qt::UserRole, static_cast<qlonglong>(layerId));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    // Color icon
    QColor color("#808080");
    if (tech_) {
        if (const auto* info = tech_->findLayerByName(layer->name())) {
            color = QColor(QString::fromStdString(info->color()));
        }
    }
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    item->setIcon(QIcon(pixmap));
  }
}

db::DbId LayerPaletteWidget::selectedLayer() const {
  auto* item = list_->currentItem();
  if (!item) return db::kInvalidId;
  return static_cast<db::DbId>(item->data(Qt::UserRole).toLongLong());
}

bool LayerPaletteWidget::isLayerVisible(db::DbId id) const {
  for (int i = 0; i < list_->count(); ++i) {
    auto* item = list_->item(i);
    if (static_cast<db::DbId>(item->data(Qt::UserRole).toLongLong()) == id) {
      return item->checkState() == Qt::Checked;
    }
  }
  return true;
}

void LayerPaletteWidget::onItemChanged(QListWidgetItem* item) {
  emit layerVisibilityChanged(static_cast<db::DbId>(item->data(Qt::UserRole).toLongLong()),
                              item->checkState() == Qt::Checked);
}

void LayerPaletteWidget::onSelectionChanged() {
  emit layerSelectionChanged(selectedLayer());
}

}  // namespace aurora::ui
