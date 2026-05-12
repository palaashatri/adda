#pragma once

#include "db/DbTypes.h"
#include "db/DbView.h"
#include "geom/GeomPoint.h"
#include "schematic/SchWire.h"

#include <map>
#include <string>
#include <vector>

namespace aurora::schematic {

struct SchNetLabel {
  db::DbId netId;
  geom::GeomPoint position;
};

struct SchStimulus {
  std::string type;           // "vdc", "idc", "vpulse", "vsin"
  db::DbId netId;
  geom::GeomPoint position;
  std::map<std::string, std::string> params;
};

class SchDocument {
 public:
  explicit SchDocument(db::DbView& view);

  [[nodiscard]] db::DbView& view();
  [[nodiscard]] const db::DbView& view() const;

  [[nodiscard]] SchWire& addWire(db::DbId netId, std::vector<geom::GeomPoint> points,
                                   bool isBus = false);
  void removeWireAt(std::size_t index);
  void clearWires();
  [[nodiscard]] const std::vector<SchWire>& wires() const;
  [[nodiscard]] std::vector<SchWire>& wires();

  [[nodiscard]] SchNetLabel& addNetLabel(db::DbId netId, geom::GeomPoint position);
  void removeNetLabelAt(std::size_t index);
  void clearNetLabels();
  [[nodiscard]] const std::vector<SchNetLabel>& netLabels() const;
  [[nodiscard]] std::vector<SchNetLabel>& netLabels();

  [[nodiscard]] SchStimulus& addStimulus(std::string type, db::DbId netId,
                                          geom::GeomPoint position);
  void removeStimulusAt(std::size_t index);
  void clearStimuli();
  [[nodiscard]] const std::vector<SchStimulus>& stimuli() const;
  [[nodiscard]] std::vector<SchStimulus>& stimuli();

 private:
  db::DbView* view_{nullptr};
  std::vector<SchWire> wires_;
  std::vector<SchNetLabel> netLabels_;
  std::vector<SchStimulus> stimuli_;
};

}  // namespace aurora::schematic
