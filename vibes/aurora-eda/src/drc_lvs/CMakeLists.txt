target_sources(
  aurora_core
  PRIVATE
    DrcEngine.cpp
    LvsChecker.cpp
    ParasiticExtractor.cpp
    ParasiticReducer.cpp
    PercChecker.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      DrcViolation.h
      DrcEngine.h
      LvsChecker.h
      ParasiticExtractor.h
      ParasiticReducer.h
      PercChecker.h
)
