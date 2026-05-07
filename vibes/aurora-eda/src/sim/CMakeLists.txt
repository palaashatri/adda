target_sources(
  aurora_core
  PRIVATE
    SimRunner.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      SimResult.h
      SimRunner.h
)
