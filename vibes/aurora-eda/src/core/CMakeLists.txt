target_sources(
  aurora_core
  PRIVATE
    CoreApp.cpp
    DesignServices.cpp
    LibraryManager.cpp
    PluginManager.cpp
    ProjectManager.cpp
    ScriptEngine.cpp
    WorkspaceServices.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      CoreApp.h
      DesignServices.h
      LibraryManager.h
      PluginApi.h
      PluginManager.h
      ProjectManager.h
      ScriptEngine.h
      WorkspaceServices.h
)

target_link_libraries(aurora_core PUBLIC ${CMAKE_DL_LIBS})
