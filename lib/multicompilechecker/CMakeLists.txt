add_library(multicompilechecker MODULE
  MultiCompileChecker.cpp
)

target_include_directories(multicompilechecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS multicompilechecker DESTINATION lib)

