add_library(mainchecker MODULE
  MainChecker.cpp
)

target_include_directories(mainchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS mainchecker DESTINATION lib)

