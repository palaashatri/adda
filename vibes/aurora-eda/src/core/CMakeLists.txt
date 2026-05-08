target_sources(
  aurora_core
  PRIVATE
    CoreApp.cpp
    PluginManager.cpp
    ProjectManager.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      CoreApp.h
      PluginApi.h
      PluginManager.h
      ProjectManager.h
)

target_link_libraries(aurora_core PUBLIC ${CMAKE_DL_LIBS})
