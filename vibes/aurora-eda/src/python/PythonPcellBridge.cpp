#include "python/PythonPcellBridge.h"

#include "db/DbView.h"
#include "pdk/PcellDescriptor.h"
#include "pdk/PcellRegistry.h"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace aurora::python {

// Import a Python module and return the class object.
static py::object importClass(const std::string& modulePath,
                              const std::string& className) {
  auto mod = py::module_::import(modulePath.c_str());
  return mod.attr(className.c_str());
}

// Convert a C++ ParamMap to a Python dict.
static py::dict toPyDict(const std::map<std::string, std::string>& params) {
  py::dict d;
  for (const auto& [k, v] : params)
    d[py::str(k)] = py::str(v);
  return d;
}

pdk::PcellGenerator makePythonPcellGenerator(const std::string& modulePath,
                                              const std::string& className) {
  return [modulePath, className](db::DbView& view, const db::DbCellLib& lib,
                                  const tech::TechDatabase& tech,
                                  const pdk::ParamMap& params) {
    try {
      py::gil_scoped_acquire acquire;
      auto cls = importClass(modulePath, className);
      auto pyView = py::cast(&view, py::return_value_policy::reference);
      auto pyTech = py::cast(&tech, py::return_value_policy::reference);
      py::dict pyParams = toPyDict(params);
      cls.attr("generate_layout")(*pyView, *pyTech, pyParams);
    } catch (const std::exception& e) {
      throw std::runtime_error(
          "Python PCell " + modulePath + "." + className +
          " failed: " + e.what());
    }
  };
}

// Register Python PCells into the PcellRegistry. Called during startup.
void registerPythonPcells(pdk::PcellRegistry& registry) {
  try {
    py::gil_scoped_acquire acquire;
    auto auroraPkg = py::module_::import("aurora.pdk.registry");
    auto registered = auroraPkg.attr("get_all_pcells")();
    for (const auto& item : registered) {
      auto t = item.cast<py::tuple>();
      std::string name = t[0].cast<std::string>();
      std::string modulePath = t[1].cast<std::string>();
      std::string className = t[2].cast<std::string>();
      registry.registerPcell(
          pdk::PcellDescriptor{name, {},
                               makePythonPcellGenerator(modulePath, className)});
    }
  } catch (const std::exception&) {
    // Python PCell registration is best-effort. If no Python PCells are
    // defined, this is a no-op.
  }
}

}  // namespace aurora::python
