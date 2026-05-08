#include "core/ProjectManager.h"

#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "db/DbInstance.h"
#include "db/DbNet.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace aurora::core {

// ─── Serialization helpers ───────────────────────────────────────────────────

static json serializeView(const db::DbView& view) {
  json jv;
  jv["id"]   = view.id();
  jv["type"] = static_cast<int>(view.type());

  // Shapes
  json shapes = json::array();
  for (const auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s) continue;
    json js;
    js["id"]      = s->id();
    js["layerId"] = s->layerId();
    js["kind"]    = static_cast<int>(s->kind());
    switch (s->kind()) {
      case db::DbShapeKind::Rect: {
        const auto& b = static_cast<const db::DbRect*>(s)->box();
        js["l"] = b.left(); js["b"] = b.bottom();
        js["r"] = b.right(); js["t"] = b.top();
        break;
      }
      case db::DbShapeKind::Polygon: {
        json pts = json::array();
        for (const auto& p : static_cast<const db::DbPolygon*>(s)->polygon().points())
          pts.push_back({{"x", p.x}, {"y", p.y}});
        js["points"] = pts;
        break;
      }
      case db::DbShapeKind::Path: {
        const auto& path = static_cast<const db::DbPath*>(s)->path();
        js["width"] = path.width();
        json pts = json::array();
        for (const auto& p : path.points()) pts.push_back({{"x", p.x}, {"y", p.y}});
        js["points"] = pts;
        break;
      }
      case db::DbShapeKind::Text: {
        const auto* t = static_cast<const db::DbText*>(s);
        js["text"] = t->text();
        js["ox"] = t->origin().x; js["oy"] = t->origin().y;
        break;
      }
    }
    shapes.push_back(js);
  }
  jv["shapes"] = shapes;

  // Nets
  json nets = json::array();
  for (const auto nid : view.netIds()) {
    const auto* n = view.findNet(nid);
    if (!n) continue;
    nets.push_back({{"id", n->id()}, {"name", n->name()}});
  }
  jv["nets"] = nets;

  // Instances
  json insts = json::array();
  for (const auto iid : view.instanceIds()) {
    const auto* inst = view.findInstance(iid);
    if (!inst) continue;
    json ji;
    ji["id"]           = inst->id();
    ji["name"]         = inst->name();
    ji["masterCellId"] = inst->masterCellId();
    ji["dx"]  = inst->transform().dx;
    ji["dy"]  = inst->transform().dy;
    ji["rot"] = inst->transform().rotationDegrees;
    ji["mirX"]= inst->transform().mirrorX;
    json params = json::object();
    for (const auto& [k, v] : inst->parameters()) params[k] = v;
    ji["params"] = params;
    insts.push_back(ji);
  }
  jv["instances"] = insts;

  return jv;
}

static json serializeCell(const db::DbCell& cell) {
  json jc;
  jc["id"]   = cell.id();
  jc["name"] = cell.name();
  json views = json::array();
  for (const auto vid : cell.viewIds()) {
    const auto* v = cell.findViewById(vid);
    if (v) views.push_back(serializeView(*v));
  }
  jc["views"] = views;
  return jc;
}

static json serializeLib(const db::DbCellLib& lib) {
  json jl;
  jl["name"] = lib.name();

  // Layers
  json layers = json::array();
  for (const auto lid : lib.layerIds()) {
    const auto* layer = lib.findLayer(lid);
    if (!layer) continue;
    layers.push_back({
      {"id",   layer->id()},
      {"name", layer->name()},
      {"purpose", layer->purpose()},
      {"color", layer->color()},
      {"gdsLayer", layer->gdsLayer()},
      {"gdsDatatype", layer->gdsDatatype()}
    });
  }
  jl["layers"] = layers;

  // Cells
  json cells = json::array();
  for (const auto cid : lib.cellIds()) {
    const auto* cell = lib.findCellById(cid);
    if (cell) cells.push_back(serializeCell(*cell));
  }
  jl["cells"] = cells;
  return jl;
}

// ─── ProjectManager methods ───────────────────────────────────────────────────

bool ProjectManager::createProject(const std::filesystem::path& projectPath) {
  std::error_code error;
  std::filesystem::create_directories(projectPath / "libraries", error);
  if (error) return false;
  std::filesystem::create_directories(projectPath / "pdk", error);
  if (error) return false;
  std::filesystem::create_directories(projectPath / "config", error);
  if (error) return false;

  currentProjectPath_ = std::filesystem::absolute(projectPath);
  return saveProject();
}

bool ProjectManager::openProject(const std::filesystem::path& projectPath) {
  if (!std::filesystem::exists(projectPath) || !std::filesystem::is_directory(projectPath))
    return false;
  currentProjectPath_ = std::filesystem::absolute(projectPath);

  // Try to load saved design
  const auto designFile = *currentProjectPath_ / "config" / "design.json";
  if (std::filesystem::exists(designFile)) {
    std::ifstream f(designFile);
    if (f) {
      try {
        const auto j = json::parse(f);
        if (j.contains("library")) {
          const auto& jl = j["library"];
          workingLibrary_ = db::DbCellLib(jl.value("name", "worklib"));
          // Layers
          for (const auto& jlay : jl.value("layers", json::array())) {
            auto& layer = workingLibrary_.createLayer(
                jlay.value("name", ""), jlay.value("purpose", "drawing"));
            layer.setColor(jlay.value("color", "#808080"));
            layer.setGdsMapping(jlay.value("gdsLayer", 0), jlay.value("gdsDatatype", 0));
          }
          // Cells (first pass — create stubs so IDs can be resolved)
          for (const auto& jcell : jl.value("cells", json::array())) {
            auto& cell = workingLibrary_.createCell(jcell.value("name", "untitled"));
            for (const auto& jview : jcell.value("views", json::array())) {
              const auto type = static_cast<db::DbViewType>(jview.value("type", 0));
              auto& view = cell.createView(type);
              // Nets
              for (const auto& jnet : jview.value("nets", json::array()))
                view.createNet(jnet.value("name", ""));
              // Shapes
              for (const auto& js : jview.value("shapes", json::array())) {
                const auto lid = static_cast<db::DbId>(js.value("layerId", 0ULL));
                const auto kind = static_cast<db::DbShapeKind>(js.value("kind", 0));
                switch (kind) {
                  case db::DbShapeKind::Rect: {
                    geom::GeomBox box{js["l"].get<long long>(), js["b"].get<long long>(),
                                      js["r"].get<long long>(), js["t"].get<long long>()};
                    view.createRect(lid, box);
                    break;
                  }
                  case db::DbShapeKind::Text: {
                    geom::GeomPoint origin{js.value("ox", 0LL), js.value("oy", 0LL)};
                    view.createText(lid, origin, js.value("text", ""));
                    break;
                  }
                  default: break;
                }
              }
              // Instances
              for (const auto& ji : jview.value("instances", json::array())) {
                db::DbTransform xform;
                xform.dx = ji.value("dx", 0LL);
                xform.dy = ji.value("dy", 0LL);
                xform.rotationDegrees = ji.value("rot", 0);
                xform.mirrorX = ji.value("mirX", false);
                const auto masterId = static_cast<db::DbId>(ji.value("masterCellId", 0ULL));
                auto& inst = view.createInstance(ji.value("name", ""), masterId, xform);
                for (const auto& [k, v] : ji.value("params", json::object()).items())
                  inst.setParameter(k, v.get<std::string>());
              }
            }
          }
        }
      } catch (...) { /* design.json parse error — start fresh */ }
    }
  }
  return true;
}

bool ProjectManager::saveProject() const {
  if (!currentProjectPath_) return false;

  const auto configDir = *currentProjectPath_ / "config";
  std::error_code error;
  std::filesystem::create_directories(configDir, error);
  if (error) return false;

  // Write project manifest
  {
    std::ofstream manifest(configDir / "project.json");
    if (!manifest) return false;
    json j;
    j["format"] = "aurora-project";
    j["version"] = 2;
    j["working_library"] = workingLibrary_.name();
    manifest << j.dump(2) << '\n';
  }

  // Write full design
  {
    std::ofstream design(configDir / "design.json");
    if (!design) return false;
    json j;
    j["format"] = "aurora-design";
    j["version"] = 2;
    j["library"] = serializeLib(workingLibrary_);
    design << j.dump(2) << '\n';
  }
  return true;
}

void ProjectManager::closeProject() { currentProjectPath_.reset(); }

bool ProjectManager::hasOpenProject() const { return currentProjectPath_.has_value(); }

const std::optional<std::filesystem::path>& ProjectManager::currentProjectPath() const {
  return currentProjectPath_;
}

void ProjectManager::addLibrarySearchPath(std::filesystem::path path) {
  librarySearchPaths_.push_back(std::move(path));
}

const std::vector<std::filesystem::path>& ProjectManager::librarySearchPaths() const {
  return librarySearchPaths_;
}

db::DbCellLib& ProjectManager::workingLibrary() { return workingLibrary_; }
const db::DbCellLib& ProjectManager::workingLibrary() const { return workingLibrary_; }

}  // namespace aurora::core
