add_library(expchecker MODULE
  ExpChecker.cpp
)

target_include_directories(expchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS expchecker DESTINATION lib)

