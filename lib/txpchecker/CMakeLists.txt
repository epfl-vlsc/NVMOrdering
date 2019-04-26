add_library(txpchecker MODULE
  TxpChecker.cpp
)

target_include_directories(txpchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS txpchecker DESTINATION lib)

