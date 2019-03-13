add_library(orderingchecker MODULE
  OrderingBugReporter.cpp
  OrderingChecker.cpp
)

target_include_directories(orderingchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS orderingchecker DESTINATION lib)

