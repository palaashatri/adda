cmake_minimum_required(VERSION 3.22)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "aurora-eda requires an out-of-source build. Use: cmake -S . -B build")
endif()

project(aurora-eda LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(AURORA_BUILD_UI "Build the Qt Widgets UI when Qt 6 is available" ON)
option(AURORA_BUILD_PYTHON "Build Python bindings when pybind11 is available" ON)
option(AURORA_BUILD_TESTS "Build aurora-eda tests" ON)

include(GNUInstallDirs)

add_subdirectory(src)

if(AURORA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
