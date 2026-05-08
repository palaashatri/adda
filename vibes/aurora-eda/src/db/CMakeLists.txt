target_sources(
  aurora_core
  PRIVATE
    DbCell.cpp
    DbCellLib.cpp
    DbConstraint.cpp
    DbInstance.cpp
    DbLayer.cpp
    DbNet.cpp
    DbPin.cpp
    DbShape.cpp
    DbView.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      DbCell.h
      DbCellLib.h
      DbConstraint.h
      DbInstance.h
      DbLayer.h
      DbNet.h
      DbPin.h
      DbShape.h
      DbTypes.h
      DbView.h
)
