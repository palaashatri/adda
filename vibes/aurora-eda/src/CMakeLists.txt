add_library(aurora_core)
add_library(aurora::core ALIAS aurora_core)

target_compile_features(aurora_core PUBLIC cxx_std_20)
target_include_directories(
  aurora_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include(CheckCXXCompilerFlag)
if(MSVC)
  target_compile_options(aurora_core PRIVATE /W4)
else()
  target_compile_options(aurora_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(geom)
add_subdirectory(db)
add_subdirectory(tech)
add_subdirectory(pdk)
add_subdirectory(netlist)
add_subdirectory(sim)
add_subdirectory(drc_lvs)
add_subdirectory(core)
add_subdirectory(schematic)
add_subdirectory(layout)
add_subdirectory(ui)
add_subdirectory(python)
add_subdirectory(plugins)
