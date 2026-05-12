target_sources(
  aurora_core
  PRIVATE
    MosPcell.cpp
    PcellRegistry.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      MosPcell.h
      PcellDescriptor.h
      PcellRegistry.h
)
