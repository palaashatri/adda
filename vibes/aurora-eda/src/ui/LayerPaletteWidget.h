#pragma once

#include "db/DbTypes.h"
#include <QWidget>

class QListWidget;
class QListWidgetItem;

namespace aurora::db { class DbCellLib; }
namespace aurora::tech { class TechDatabase; }

namespace aurora::ui {

class LayerPaletteWidget : public QWidget {
  Q_OBJECT

 public:
  explicit LayerPaletteWidget(QWidget* parent = nullptr);

  void setLibrary(const db::DbCellLib* lib, const tech::TechDatabase* tech);
  
  [[nodiscard]] db::DbId selectedLayer() const;
  [[nodiscard]] bool isLayerVisible(db::DbId id) const;

 signals:
  void layerSelectionChanged(db::DbId id);
  void layerVisibilityChanged(db::DbId id, bool visible);

 private slots:
  void onItemChanged(QListWidgetItem* item);
  void onSelectionChanged();

 private:
  QListWidget* list_{nullptr};
  const db::DbCellLib* lib_{nullptr};
  const tech::TechDatabase* tech_{nullptr};
};

}  // namespace aurora::ui
