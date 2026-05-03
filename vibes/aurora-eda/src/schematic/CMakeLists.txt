add_library(aurora_schematic)
add_library(aurora::schematic ALIAS aurora_schematic)

target_sources(
  aurora_schematic
  PRIVATE
    SchDocument.cpp
    SchEditorController.cpp
    SchSymbol.cpp
    SchWire.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      SchDocument.h
      SchEditorController.h
      SchSymbol.h
      SchWire.h
)

target_link_libraries(aurora_schematic PUBLIC aurora_core)
target_compile_features(aurora_schematic PUBLIC cxx_std_20)
