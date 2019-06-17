add_library(ptrchecker MODULE
  PtrChecker.cpp
)

target_include_directories(ptrchecker
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS ptrchecker DESTINATION lib)

