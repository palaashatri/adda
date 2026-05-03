find_package(nlohmann_json CONFIG REQUIRED)

target_sources(
  aurora_core
  PRIVATE
    TechDatabase.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      TechDatabase.h
)

target_link_libraries(aurora_core PRIVATE nlohmann_json::nlohmann_json)
