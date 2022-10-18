cmake_minimum_required(VERSION 3.14.0)
project(kmindex VERSION 0.0.1 LANGUAGES C CXX)

set(PROJECT_DESCRIPTION "kmindex")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE="Release")
endif()

set(CMAKE_CXX_STANDARD 17)

add_library(build_type_flags INTERFACE)
target_compile_options(build_type_flags INTERFACE
  $<$<CONFIG:Debug>:-O0 -g>
  $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
  $<$<CONFIG:Profile>:-O3 -ggdb3 -DNDEBUG -fno-inline>
  $<$<CONFIG:Coverage>:-O0 -g --coverage>
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
add_library(opti_flag INTERFACE)
target_compile_options(opti_flag INTERFACE
  $<IF:$<BOOL:${PORTABLE_BUILD}>,-march=x86-64,-march=native>
)

add_subdirectory(thirdparty)
add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(tests)



