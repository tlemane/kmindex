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
  spdlog
  semver
  atomic_queue
  zstd_ext
  bitmatrixshuffle
)
if (STATIC_BUILD)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    -Wl,--whole-archive
    pthread
    -Wl,--no-whole-archive
  )
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC pthread)
endif()


if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0.0)
  target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
endif()

if (ipo_supported)
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if (STATIC_BUILD AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
  target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()

add_dependencies(${PROJECT_NAME} deps)

install (TARGETS ${PROJECT_NAME}
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         COMPONENT ${KMINDEX_LIB}_Runtime
)

