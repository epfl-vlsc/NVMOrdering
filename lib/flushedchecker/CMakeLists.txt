add_library(flushedchecker MODULE
  FlushedChecker.cpp
)

target_include_directories(flushedchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS flushedchecker DESTINATION lib)

