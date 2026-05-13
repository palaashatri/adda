add_library(aurora_layout)
add_library(aurora::layout ALIAS aurora_layout)

target_sources(
  aurora_layout
  PRIVATE
    ImportWizard.cpp
    LayCifIo.cpp
    LayDocument.cpp
    LayDxfWriter.cpp
    LayEditorController.cpp
    LayDefReader.cpp
    LayGdsReader.cpp
    LayDefWriter.cpp
    LayGdsWriter.cpp
    LayImageExport.cpp
    LayLefReader.cpp
    LayLefWriter.cpp
    LayOasisReader.cpp
    LayOasisWriter.cpp
    LayTool.cpp
    LayToolGuardRing.cpp
    LayToolPath.cpp
    LayToolPolygon.cpp
    LayToolRect.cpp
    LayToolRuler.cpp
    LayToolSelect.cpp
    LayToolStretch.cpp
    LayToolVia.cpp
    LayToolViaArray.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      ImportWizard.h
      LayCifIo.h
      LayDocument.h
      LayDxfWriter.h
      LayEditorController.h
      LayDefReader.h
      LayGdsReader.h
      LayDefWriter.h
      LayGdsWriter.h
      LayImageExport.h
      LayLefReader.h
      LayLefWriter.h
      LayOasisReader.h
      LayOasisWriter.h
      LayTool.h
      LayToolGuardRing.h
      LayToolPath.h
      LayToolPolygon.h
      LayToolRect.h
      LayToolRuler.h
      LayToolSelect.h
      LayToolStretch.h
      LayToolVia.h
      LayToolViaArray.h
)

target_link_libraries(aurora_layout PUBLIC aurora_core)
target_compile_features(aurora_layout PUBLIC cxx_std_20)
