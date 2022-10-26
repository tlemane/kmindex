file(GLOB_RECURSE KMINDEX_SERVER_SOURCES "*.cpp")

find_package(Boost 1.70.0 REQUIRED COMPONENTS system)
set(KMINDEX_SERVER "${PROJECT_NAME}-server")
add_executable(${KMINDEX_SERVER} ${KMINDEX_SERVER_SOURCES})

target_compile_features(${KMINDEX_SERVER} PUBLIC cxx_std_17)
target_link_libraries(${KMINDEX_SERVER} PUBLIC
  $<BUILD_INTERFACE:build_type_flags>
  $<BUILD_INTERFACE:warning_flags>
  $<BUILD_INTERFACE:asan_flags>
  Boost::boost
  Boost::system
  bcli_lib
  kmindex-lib
  kseqpp
  zlib
  pthread
  spdlog
  sws
)
add_dependencies(${KMINDEX_SERVER} deps)

