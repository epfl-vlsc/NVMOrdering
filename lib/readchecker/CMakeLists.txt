add_library(readchecker MODULE
  ReadChecker.cpp
)

target_include_directories(readchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS readchecker DESTINATION lib)

