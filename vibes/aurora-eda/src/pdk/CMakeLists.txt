target_sources(
  aurora_core
  PRIVATE
    PcellRegistry.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      PcellDescriptor.h
      PcellRegistry.h
)
