file(GLOB_RECURSE KMINDEX_SOURCES "*.cpp")
add_executable(${PROJECT_NAME} ${KMINDEX_SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:build_type_flags>
  $<BUILD_INTERFACE:warning_flags>
  $<BUILD_INTERFACE:asan_flags>
  bcli_lib
  kmindex-lib
  kseqpp
  zlib
  pthread
  spdlog
)
add_dependencies(${PROJECT_NAME} deps)

install (TARGETS ${PROJECT_NAME}
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         COMPONENT ${KMINDEX_LIB}_Runtime
)

