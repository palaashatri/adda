#include "core/CoreApp.h"
#include "db/DbCell.h"
#include "db/DbCellLib.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "tech/TechDatabase.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(aurora, m) {
  m.doc() = "aurora-eda Python bindings";

  py::class_<aurora::core::CoreApp>(m, "CoreApp")
      .def(py::init<>())
      .def("initialize", &aurora::core::CoreApp::initialize, py::arg("plugin_directory") = "")
      .def("shutdown", &aurora::core::CoreApp::shutdown)
      .def_property_readonly("initialized", &aurora::core::CoreApp::initialized);

  py::enum_<aurora::db::DbViewType>(m, "ViewType")
      .value("SCHEMATIC", aurora::db::DbViewType::Schematic)
      .value("SYMBOL", aurora::db::DbViewType::Symbol)
      .value("LAYOUT", aurora::db::DbViewType::Layout)
      .value("ABSTRACT", aurora::db::DbViewType::Abstract)
      .value("NETLIST", aurora::db::DbViewType::Netlist);

  py::class_<aurora::db::DbCellLib>(m, "CellLib")
      .def(py::init<std::string>())
      .def_property("name", &aurora::db::DbCellLib::name, &aurora::db::DbCellLib::setName)
      .def("create_cell", &aurora::db::DbCellLib::createCell, py::return_value_policy::reference)
      .def("cell_ids", &aurora::db::DbCellLib::cellIds);

  py::class_<aurora::db::DbCell>(m, "Cell")
      .def_property("name", &aurora::db::DbCell::name, &aurora::db::DbCell::setName)
      .def_property_readonly("id", &aurora::db::DbCell::id)
      .def("create_view", &aurora::db::DbCell::createView, py::return_value_policy::reference)
      .def("view_ids", &aurora::db::DbCell::viewIds);

  py::class_<aurora::db::DbView>(m, "View")
      .def_property_readonly("id", &aurora::db::DbView::id)
      .def_property_readonly("type", &aurora::db::DbView::type)
      .def("shape_ids", &aurora::db::DbView::shapeIds);

  py::class_<aurora::tech::TechDatabase>(m, "TechDatabase")
      .def(py::init<>())
      .def("load_from_json_file", &aurora::tech::TechDatabase::loadFromJsonFile)
      .def_property_readonly("loaded", &aurora::tech::TechDatabase::loaded);
}
