add_library(mainchecker MODULE
  MainChecker.cpp
)

add_library(recchecker MODULE
  RecChecker.cpp
)

target_include_directories(mainchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(recchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS mainchecker DESTINATION lib)

install(TARGETS recchecker DESTINATION lib)

