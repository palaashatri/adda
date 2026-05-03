target_sources(
  aurora_core
  PRIVATE
    GeomBox.cpp
    GeomOps.cpp
    GeomPath.cpp
    GeomPoint.cpp
    GeomPolygon.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      GeomBox.h
      GeomOps.h
      GeomPath.h
      GeomPoint.h
      GeomPolygon.h
)
