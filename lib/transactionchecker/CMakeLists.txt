add_library(transactionchecker MODULE
  TransactionChecker.cpp
)

target_include_directories(transactionchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS transactionchecker DESTINATION lib)

