add_library(aurora_layout)
add_library(aurora::layout ALIAS aurora_layout)

target_sources(
  aurora_layout
  PRIVATE
    LayDocument.cpp
    LayEditorController.cpp
    LayTool.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      LayDocument.h
      LayEditorController.h
      LayTool.h
)

target_link_libraries(aurora_layout PUBLIC aurora_core)
target_compile_features(aurora_layout PUBLIC cxx_std_20)
