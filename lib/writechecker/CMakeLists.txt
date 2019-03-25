add_library(writechecker MODULE
  WriteChecker.cpp
)

target_include_directories(writechecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS writechecker DESTINATION lib)

