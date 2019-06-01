add_library(dfmainchecker MODULE
  DfMainChecker.cpp
)

target_include_directories(dfmainchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS dfmainchecker DESTINATION lib)

