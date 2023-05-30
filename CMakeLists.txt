cmake_minimum_required(VERSION 3.24.0)
project(kmindex VERSION 0.4.0 LANGUAGES C CXX)

set(PROJECT_DESCRIPTION "kmindex")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

option(PORTABLE_BUILD "Generic x86_64 build" OFF)
option(WITH_APP "Compile kmindex cli" ON)
option(WITH_SERVER "Compile kmindex-server" ON)
option(WITH_TESTS "Compile tests" OFF)
option(STATIC_BUILD "Static build" OFF)

if (NOT MAX_KMER_SIZE)
  set(MAX_KMER_SIZE 32)
else()
  math(EXPR RESULT "(${MAX_KMER_SIZE} % 32)")
  if (${RESULT})
    message(FATAL_ERROR "MAX_KMER_SIZE should be a multiple of 32.")
  endif()
endif()

add_compile_definitions(MAX_KMER_SIZE=${MAX_KMER_SIZE})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE="Release")
endif()

set(CMAKE_CXX_STANDARD 17)

add_library(build_type_flags INTERFACE)
target_compile_options(build_type_flags INTERFACE
  $<$<CONFIG:Debug>:-O -g>
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
  $<$<CONFIG:Profile>:-O3 -ggdb3 -DNDEBUG -fno-inline>
  $<$<CONFIG:Coverage>:-O0 -g --coverage>
  $<IF:$<BOOL:${PORTABLE_BUILD}>,-march=x86-64,-march=native>
)

add_library(warning_flags INTERFACE)
target_compile_options(warning_flags INTERFACE
  "-Wall"
  "-Wextra"
  "-pedantic"
  $<$<CONFIG:Release>:
   $<$<BOOL:${UNNAMED_WERROR}>:-Werror>>
)

add_library(asan_flags INTERFACE)
target_compile_options(asan_flags INTERFACE
  $<$<CONFIG:Debug>:-fno-omit-frame-pointer -fsanitize=address>
)
target_link_options(asan_flags INTERFACE
  $<$<CONFIG:Debug>:-fno-omit-frame-pointer -fsanitize=address>
)
target_link_libraries(asan_flags INTERFACE
  $<$<CONFIG:Debug>:asan>
)

add_subdirectory(thirdparty)
add_subdirectory(lib)
add_subdirectory(app)

if (WITH_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

include(CPackConfig)


