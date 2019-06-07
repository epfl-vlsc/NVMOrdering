add_library(pairchecker MODULE
  PairChecker.cpp
)

target_include_directories(pairchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS pairchecker DESTINATION lib)

