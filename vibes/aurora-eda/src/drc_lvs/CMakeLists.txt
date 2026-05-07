target_sources(
  aurora_core
  PRIVATE
    DrcEngine.cpp
    LvsChecker.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      DrcViolation.h
      DrcEngine.h
      LvsChecker.h
)
