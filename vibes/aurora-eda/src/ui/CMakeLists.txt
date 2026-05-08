if(AURORA_BUILD_UI)
  find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL QUIET)
endif()

if(AURORA_BUILD_UI AND Qt6_FOUND)
  add_library(aurora_ui)
  add_library(aurora::ui ALIAS aurora_ui)

  target_sources(
    aurora_ui
    PRIVATE
      LayerPaletteWidget.cpp
      LayoutViewWidget.cpp
      MainWindow.cpp
      PropertyEditorWidget.cpp
      SchematicViewWidget.cpp
    PUBLIC
      FILE_SET HEADERS
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
      FILES
        LayerPaletteWidget.h
        LayoutViewWidget.h
        MainWindow.h
        PropertyEditorWidget.h
        SchematicViewWidget.h
  )

  target_link_libraries(
    aurora_ui
    PUBLIC
      aurora_core
      aurora_schematic
      aurora_layout
      Qt6::Core
      Qt6::Gui
      Qt6::Widgets
      Qt6::OpenGL
  )
  target_compile_features(aurora_ui PUBLIC cxx_std_20)

  add_executable(aurora_app AppMain.cpp)
  target_link_libraries(aurora_app PRIVATE aurora_ui Qt6::Core Qt6::Gui Qt6::Widgets)
  target_compile_features(aurora_app PRIVATE cxx_std_20)
  set_target_properties(aurora_app PROPERTIES OUTPUT_NAME aurora-eda)
else()
  add_library(aurora_ui UiAvailability.cpp)
  add_library(aurora::ui ALIAS aurora_ui)
  target_link_libraries(aurora_ui PUBLIC aurora_core aurora_schematic aurora_layout)
  target_compile_definitions(aurora_ui PUBLIC AURORA_UI_STUB=1)

  add_executable(aurora_app AppMainStub.cpp)
  target_link_libraries(aurora_app PRIVATE aurora_ui)
  target_compile_features(aurora_app PRIVATE cxx_std_20)
  target_compile_definitions(aurora_app PRIVATE AURORA_UI_STUB=1)
  set_target_properties(aurora_app PROPERTIES OUTPUT_NAME aurora-eda)
endif()
