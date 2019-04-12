add_library(txmchecker MODULE
  TxmChecker.cpp
)

target_include_directories(txmchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS txmchecker DESTINATION lib)

