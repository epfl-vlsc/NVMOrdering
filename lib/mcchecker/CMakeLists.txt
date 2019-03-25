add_library(mcchecker MODULE
  MultiCompileChecker.cpp
)

target_include_directories(mcchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS mcchecker DESTINATION lib)

