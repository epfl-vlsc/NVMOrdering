add_library(recoverychecker MODULE
  RecoveryChecker.cpp
)

target_include_directories(recoverychecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS recoverychecker DESTINATION lib)

