add_library(txpchecker MODULE
  TxPChecker.cpp
)

target_include_directories(txpchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS txpchecker DESTINATION lib)

