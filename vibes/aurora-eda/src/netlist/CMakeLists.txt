target_sources(
  aurora_core
  PRIVATE
    NetlistGenerator.cpp
    SpiceImporter.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      NetlistGenerator.h
      SpiceImporter.h
)
