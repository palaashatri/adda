#pragma once

#include "drc_lvs/DrcViolation.h"
#include "drc_lvs/LvsChecker.h"

#include <QDialog>

#include <vector>

class QLabel;
class QPushButton;
class QTableWidget;
class QTabWidget;

namespace aurora {
namespace db { class DbCellLib; class DbView; }
namespace tech { class TechDatabase; }
}  // namespace aurora

namespace aurora::ui {

class LayoutViewWidget;

class DrcResultsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit DrcResultsDialog(QWidget* parent = nullptr);

  void setLayoutView(const db::DbView* view, const db::DbCellLib* lib,
                     const tech::TechDatabase* tech);
  void setSchematicView(const db::DbView* schView, const db::DbView* layView,
                        const db::DbCellLib* lib);
  void setLayoutWidget(LayoutViewWidget* layWidget) { layWidget_ = layWidget; }

 private slots:
  void onRunDrc();
  void onRunLvs();
  void onDrcRowDoubleClicked(int row, int col);

 private:
  QTabWidget*   tabs_{nullptr};

  // DRC tab
  QTableWidget* drcTable_{nullptr};
  QLabel*       drcSummary_{nullptr};

  // LVS tab
  QTableWidget* lvsTable_{nullptr};
  QLabel*       lvsSummary_{nullptr};

  const db::DbView*       layView_{nullptr};
  const db::DbCellLib*    lib_{nullptr};
  const tech::TechDatabase* tech_{nullptr};
  const db::DbView*       schView_{nullptr};

  LayoutViewWidget* layWidget_{nullptr};

  std::vector<drc_lvs::DrcViolation> drcViolations_;
};

}  // namespace aurora::ui
