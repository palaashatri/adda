target_sources(
  aurora_core
  PRIVATE
    CdlGenerator.cpp
    DspfGenerator.cpp
    NetlistGenerator.cpp
    SpiceImporter.cpp
    VerilogGenerator.cpp
    VerilogImporter.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      CdlGenerator.h
      DspfGenerator.h
      NetlistGenerator.h
      SpiceImporter.h
      VerilogGenerator.h
      VerilogImporter.h
)
