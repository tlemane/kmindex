file(GLOB_RECURSE KMINDEX_SERVER_SOURCES "*.cpp")

set(KMINDEX_SERVER "${PROJECT_NAME}-server")
add_executable(${KMINDEX_SERVER} ${KMINDEX_SERVER_SOURCES})

target_compile_features(${KMINDEX_SERVER} PUBLIC cxx_std_17)
target_link_libraries(${KMINDEX_SERVER} PUBLIC
  $<BUILD_INTERFACE:build_type_flags>
  $<BUILD_INTERFACE:warning_flags>
  $<BUILD_INTERFACE:asan_flags>
  bcli_lib
  kmindex-lib
  kseqpp
  zlib
  pthread
  spdlog
  sws
)


