target_sources(
  aurora_core
  PRIVATE
    NetlistGenerator.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      NetlistGenerator.h
)
