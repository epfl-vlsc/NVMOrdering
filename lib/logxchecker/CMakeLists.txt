add_library(logchecker MODULE
  LogChecker.cpp
)

target_include_directories(logchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS logchecker DESTINATION lib)

