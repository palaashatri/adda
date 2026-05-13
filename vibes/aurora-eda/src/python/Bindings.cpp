#include "core/CoreApp.h"
#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbConstraint.h"
#include "db/DbInstance.h"
#include "db/DbLayer.h"
#include "db/DbNet.h"
#include "db/DbPin.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"
#include "geom/GeomPath.h"
#include "geom/GeomPoint.h"
#include "geom/GeomPolygon.h"
#include "netlist/NetlistGenerator.h"
#include "tech/TechDatabase.h"

// TODO(A25): Expand Python bindings to cover DbView, DbShape, SchDocument,
// LayDocument, and other key classes. Currently only exposes basic create_*
// helpers. A full API would bind shape iteration, instance traversal, net
// queries, and tool operations so scripts can drive the EDA programmatically.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(aurora, m) {
  m.doc() = "aurora-eda Python bindings";

  // ── Geometry ─────────────────────────────────────────────────────────────

  py::class_<aurora::geom::GeomPoint>(m, "Point")
      .def(py::init<>())
      .def(py::init<aurora::geom::DbUnit, aurora::geom::DbUnit>(), py::arg("x"), py::arg("y"))
      .def_readwrite("x", &aurora::geom::GeomPoint::x)
      .def_readwrite("y", &aurora::geom::GeomPoint::y)
      .def("translated", &aurora::geom::GeomPoint::translated)
      .def("__eq__", [](const aurora::geom::GeomPoint& a, const aurora::geom::GeomPoint& b) {
        return a == b;
      })
      .def("__repr__", [](const aurora::geom::GeomPoint& p) {
        return "Point(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
      });

  py::class_<aurora::geom::GeomBox>(m, "Box")
      .def(py::init<>())
      .def(py::init<aurora::geom::DbUnit, aurora::geom::DbUnit, aurora::geom::DbUnit,
                    aurora::geom::DbUnit>(),
           py::arg("left"), py::arg("bottom"), py::arg("right"), py::arg("top"))
      .def_property_readonly("left", &aurora::geom::GeomBox::left)
      .def_property_readonly("bottom", &aurora::geom::GeomBox::bottom)
      .def_property_readonly("right", &aurora::geom::GeomBox::right)
      .def_property_readonly("top", &aurora::geom::GeomBox::top)
      .def_property_readonly("width", &aurora::geom::GeomBox::width)
      .def_property_readonly("height", &aurora::geom::GeomBox::height)
      .def_property_readonly("empty", &aurora::geom::GeomBox::empty)
      .def("contains", &aurora::geom::GeomBox::contains)
      .def("intersects", &aurora::geom::GeomBox::intersects)
      .def("__repr__", [](const aurora::geom::GeomBox& b) {
        return "Box(" + std::to_string(b.left()) + ", " + std::to_string(b.bottom()) + ", " +
               std::to_string(b.right()) + ", " + std::to_string(b.top()) + ")";
      });

  // ── Enumerations ──────────────────────────────────────────────────────────

  py::enum_<aurora::db::DbViewType>(m, "ViewType")
      .value("SCHEMATIC", aurora::db::DbViewType::Schematic)
      .value("SYMBOL", aurora::db::DbViewType::Symbol)
      .value("LAYOUT", aurora::db::DbViewType::Layout)
      .value("ABSTRACT", aurora::db::DbViewType::Abstract)
      .value("NETLIST", aurora::db::DbViewType::Netlist)
      .export_values();

  py::enum_<aurora::db::DbPinDirection>(m, "PinDirection")
      .value("INPUT", aurora::db::DbPinDirection::Input)
      .value("OUTPUT", aurora::db::DbPinDirection::Output)
      .value("INOUT", aurora::db::DbPinDirection::InOut)
      .value("PASSIVE", aurora::db::DbPinDirection::Passive)
      .value("UNKNOWN", aurora::db::DbPinDirection::Unknown)
      .export_values();

  py::enum_<aurora::db::DbShapeKind>(m, "ShapeKind")
      .value("RECT", aurora::db::DbShapeKind::Rect)
      .value("POLYGON", aurora::db::DbShapeKind::Polygon)
      .value("PATH", aurora::db::DbShapeKind::Path)
      .value("TEXT", aurora::db::DbShapeKind::Text)
      .export_values();

  // ── Database objects ──────────────────────────────────────────────────────

  py::class_<aurora::db::DbLayer>(m, "Layer")
      .def_property_readonly("id", &aurora::db::DbLayer::id)
      .def_property_readonly("name", &aurora::db::DbLayer::name)
      .def_property_readonly("purpose", &aurora::db::DbLayer::purpose)
      .def_property("color", &aurora::db::DbLayer::color, &aurora::db::DbLayer::setColor)
      .def_property_readonly("gds_layer", &aurora::db::DbLayer::gdsLayer)
      .def_property_readonly("gds_datatype", &aurora::db::DbLayer::gdsDatatype)
      .def("set_gds_mapping", &aurora::db::DbLayer::setGdsMapping, py::arg("layer"),
           py::arg("datatype"));

  py::class_<aurora::db::DbNet>(m, "Net")
      .def_property_readonly("id", &aurora::db::DbNet::id)
      .def_property("name", &aurora::db::DbNet::name, &aurora::db::DbNet::setName)
      .def_property_readonly("pin_ids", &aurora::db::DbNet::pinIds)
      .def("add_pin", &aurora::db::DbNet::addPin)
      .def("remove_pin", &aurora::db::DbNet::removePin)
      .def("set_property", &aurora::db::DbNet::setProperty);

  py::class_<aurora::db::DbPin>(m, "Pin")
      .def_property_readonly("id", &aurora::db::DbPin::id)
      .def_property("name", &aurora::db::DbPin::name, &aurora::db::DbPin::setName)
      .def_property("direction", &aurora::db::DbPin::direction, &aurora::db::DbPin::setDirection)
      .def_property("net_id", &aurora::db::DbPin::netId, &aurora::db::DbPin::setNetId)
      .def_property("instance_id", &aurora::db::DbPin::instanceId, &aurora::db::DbPin::setInstanceId)
      .def_property_readonly("shape_ids", &aurora::db::DbPin::shapeIds)
      .def("add_shape", &aurora::db::DbPin::addShape);

  py::class_<aurora::db::DbInstance>(m, "Instance")
      .def_property_readonly("id", &aurora::db::DbInstance::id)
      .def_property("name", &aurora::db::DbInstance::name, &aurora::db::DbInstance::setName)
      .def_property("master_cell_id", &aurora::db::DbInstance::masterCellId,
                    &aurora::db::DbInstance::setMasterCellId)
      .def("set_parameter", &aurora::db::DbInstance::setParameter, py::arg("name"),
           py::arg("value"))
      .def_property_readonly("parameters", &aurora::db::DbInstance::parameters);

  py::class_<aurora::db::DbShape>(m, "Shape")
      .def_property_readonly("id", &aurora::db::DbShape::id)
      .def_property("layer_id", &aurora::db::DbShape::layerId, &aurora::db::DbShape::setLayerId)
      .def_property_readonly("kind", &aurora::db::DbShape::kind);

  py::class_<aurora::db::DbRect, aurora::db::DbShape>(m, "Rect")
      .def_property_readonly("box", &aurora::db::DbRect::box)
      .def("set_box", &aurora::db::DbRect::setBox);

  py::class_<aurora::db::DbView>(m, "View")
      .def_property_readonly("id", &aurora::db::DbView::id)
      .def_property_readonly("cell_id", &aurora::db::DbView::cellId)
      .def_property_readonly("type", &aurora::db::DbView::type)
      .def("create_rect",
           [](aurora::db::DbView& v, aurora::db::DbId layerId, aurora::geom::DbUnit l,
              aurora::geom::DbUnit b, aurora::geom::DbUnit r, aurora::geom::DbUnit t)
               -> aurora::db::DbRect& {
             return v.createRect(layerId, aurora::geom::GeomBox{l, b, r, t});
           },
           py::return_value_policy::reference)
      .def("create_net", &aurora::db::DbView::createNet, py::return_value_policy::reference)
      .def("create_pin",
           [](aurora::db::DbView& v, const std::string& name, aurora::db::DbPinDirection dir,
              aurora::db::DbId netId, aurora::db::DbId instanceId) -> aurora::db::DbPin& {
             return v.createPin(name, dir, netId, instanceId);
           },
           py::arg("name"), py::arg("direction"),
           py::arg("net_id") = aurora::db::kInvalidId,
           py::arg("instance_id") = aurora::db::kInvalidId,
           py::return_value_policy::reference)
      .def("create_instance",
           [](aurora::db::DbView& v, const std::string& name,
              aurora::db::DbId masterCellId) -> aurora::db::DbInstance& {
             return v.createInstance(name, masterCellId);
           },
           py::return_value_policy::reference)
      .def("find_net", py::overload_cast<aurora::db::DbId>(&aurora::db::DbView::findNet),
           py::return_value_policy::reference)
      .def("find_pin", py::overload_cast<aurora::db::DbId>(&aurora::db::DbView::findPin),
           py::return_value_policy::reference)
      .def("find_instance",
           py::overload_cast<aurora::db::DbId>(&aurora::db::DbView::findInstance),
           py::return_value_policy::reference)
      .def("find_instance_pins",
           [](aurora::db::DbView& v, aurora::db::DbId instanceId) {
             return v.findInstancePins(instanceId);
           },
           py::return_value_policy::reference)
      .def("shape_ids", &aurora::db::DbView::shapeIds)
      .def("net_ids", &aurora::db::DbView::netIds)
      .def("pin_ids", &aurora::db::DbView::pinIds)
      .def("instance_ids", &aurora::db::DbView::instanceIds);

  py::class_<aurora::db::DbCell>(m, "Cell")
      .def_property("name", &aurora::db::DbCell::name, &aurora::db::DbCell::setName)
      .def_property_readonly("id", &aurora::db::DbCell::id)
      .def("set_parameter", &aurora::db::DbCell::setParameter)
      .def("create_view", &aurora::db::DbCell::createView, py::return_value_policy::reference)
      .def("find_view",
           py::overload_cast<aurora::db::DbViewType>(&aurora::db::DbCell::findView),
           py::return_value_policy::reference)
      .def("view_ids", &aurora::db::DbCell::viewIds);

  py::class_<aurora::db::DbCellLib>(m, "CellLib")
      .def(py::init<std::string>())
      .def_property("name", &aurora::db::DbCellLib::name, &aurora::db::DbCellLib::setName)
      .def("create_cell", &aurora::db::DbCellLib::createCell, py::return_value_policy::reference)
      .def("find_cell",
           [](aurora::db::DbCellLib& lib, const std::string& name) {
             return lib.findCell(name);
           },
           py::return_value_policy::reference)
      .def("create_layer", &aurora::db::DbCellLib::createLayer, py::return_value_policy::reference)
      .def("cell_ids", &aurora::db::DbCellLib::cellIds)
      .def("layer_ids", &aurora::db::DbCellLib::layerIds);

  // ── Technology ────────────────────────────────────────────────────────────

  py::class_<aurora::tech::TechUnits>(m, "TechUnits")
      .def_readwrite("database_unit", &aurora::tech::TechUnits::databaseUnit)
      .def_readwrite("dbu_per_micron", &aurora::tech::TechUnits::dbuPerMicron);

  py::class_<aurora::tech::TechLayerInfo>(m, "TechLayerInfo")
      .def_readwrite("id", &aurora::tech::TechLayerInfo::id)
      .def_readwrite("name", &aurora::tech::TechLayerInfo::name)
      .def_readwrite("purpose", &aurora::tech::TechLayerInfo::purpose)
      .def_readwrite("color", &aurora::tech::TechLayerInfo::color)
      .def_readwrite("gds_layer", &aurora::tech::TechLayerInfo::gdsLayer)
      .def_readwrite("gds_datatype", &aurora::tech::TechLayerInfo::gdsDatatype)
      .def_readwrite("default_width", &aurora::tech::TechLayerInfo::defaultWidth)
      .def_readwrite("default_spacing", &aurora::tech::TechLayerInfo::defaultSpacing);

  py::class_<aurora::tech::TechRule>(m, "TechRule")
      .def_readwrite("layer_name", &aurora::tech::TechRule::layerName)
      .def_readwrite("type", &aurora::tech::TechRule::type)
      .def_readwrite("value", &aurora::tech::TechRule::value)
      .def_readwrite("applies_to", &aurora::tech::TechRule::appliesTo);

  py::class_<aurora::tech::TechDatabase>(m, "TechDatabase")
      .def(py::init<>())
      .def("load_from_json_file", &aurora::tech::TechDatabase::loadFromJsonFile)
      .def_property_readonly("loaded", &aurora::tech::TechDatabase::loaded)
      .def_property_readonly("name", &aurora::tech::TechDatabase::name)
      .def_property_readonly("units", &aurora::tech::TechDatabase::units)
      .def_property_readonly("last_error", &aurora::tech::TechDatabase::lastError)
      .def("find_layer_by_name",
           [](const aurora::tech::TechDatabase& db, const std::string& name) {
             return db.findLayerByName(name);
           },
           py::return_value_policy::reference)
      .def("find_layer_by_id", &aurora::tech::TechDatabase::findLayerById,
           py::return_value_policy::reference)
      .def("layer_ids", &aurora::tech::TechDatabase::layerIds)
      .def("rules", &aurora::tech::TechDatabase::rules,
           py::return_value_policy::reference_internal)
      .def("default_width_for_layer",
           [](const aurora::tech::TechDatabase& db, const std::string& name) {
             return db.defaultWidthForLayer(name);
           })
      .def("default_spacing_for_layer",
           [](const aurora::tech::TechDatabase& db, const std::string& name) {
             return db.defaultSpacingForLayer(name);
           })
      .def("clear", &aurora::tech::TechDatabase::clear);

  // ── Netlist ───────────────────────────────────────────────────────────────

  py::class_<aurora::netlist::NetlistGenerator>(m, "NetlistGenerator")
      .def(py::init<>())
      .def("generate_spice", &aurora::netlist::NetlistGenerator::generateSpice,
           py::arg("lib"), py::arg("cell"), py::arg("view"));

  // ── CoreApp ───────────────────────────────────────────────────────────────

  py::class_<aurora::core::CoreApp>(m, "CoreApp")
      .def(py::init<>())
      .def("initialize", &aurora::core::CoreApp::initialize,
           py::arg("plugin_directory") = std::filesystem::path{})
      .def("shutdown", &aurora::core::CoreApp::shutdown)
      .def_property_readonly("initialized", &aurora::core::CoreApp::initialized)
      .def_property_readonly("version",
                             [](const aurora::core::CoreApp&) {
                               return std::string(aurora::core::CoreApp::version());
                             })
      .def_property_readonly("application_name",
                             [](const aurora::core::CoreApp&) {
                               return std::string(aurora::core::CoreApp::applicationName());
                             })
      .def("working_library",
           [](aurora::core::CoreApp& a) -> aurora::db::DbCellLib& {
             return a.projects().workingLibrary();
           },
           py::return_value_policy::reference_internal);
}
