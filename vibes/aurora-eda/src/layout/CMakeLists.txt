add_library(aurora_layout)
add_library(aurora::layout ALIAS aurora_layout)

target_sources(
  aurora_layout
  PRIVATE
    LayDocument.cpp
    LayEditorController.cpp
    LayGdsReader.cpp
    LayGdsWriter.cpp
    LayTool.cpp
    LayToolGuardRing.cpp
    LayToolPath.cpp
    LayToolPolygon.cpp
    LayToolRect.cpp
    LayToolSelect.cpp
    LayToolViaArray.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      LayDocument.h
      LayEditorController.h
      LayGdsReader.h
      LayGdsWriter.h
      LayTool.h
      LayToolGuardRing.h
      LayToolPath.h
      LayToolPolygon.h
      LayToolRect.h
      LayToolSelect.h
      LayToolViaArray.h
)

target_link_libraries(aurora_layout PUBLIC aurora_core)
target_compile_features(aurora_layout PUBLIC cxx_std_20)
