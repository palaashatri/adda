target_sources(
  aurora_core
  PRIVATE
    Cdf.cpp
    MosPcell.cpp
    PcellEvalCache.cpp
    PcellLibrary.cpp
    PcellRegistry.cpp
    PdkManager.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      Cdf.h
      MosPcell.h
      PcellDescriptor.h
      PcellEvalCache.h
      PcellLibrary.h
      PcellRegistry.h
      PdkManager.h
)
