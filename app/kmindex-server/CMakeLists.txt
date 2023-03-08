file(GLOB_RECURSE KMINDEX_SERVER_SOURCES "*.cpp")

if (STATIC_BUILD)
  set(Boost_USE_STATIC_LIBS ON)
endif()

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
  spdlog
  sws
)

if (STATIC_BUILD)
  target_link_libraries(${KMINDEX_SERVER} PUBLIC
    -Wl,--whole-archive
    pthread
    -Wl,--no-whole-archive
  )
else()
  target_link_libraries(${KMINDEX_SERVER} PUBLIC pthread)
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0.0)
  target_link_libraries(${KMINDEX_SERVER} PUBLIC stdc++fs)
endif()

if (ipo_supported)
  set_property(TARGET ${KMINDEX_SERVER} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if (STATIC_BUILD AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
  target_link_options(${KMINDEX_SERVER} PRIVATE -static)
endif()

add_dependencies(${KMINDEX_SERVER} deps)

install (TARGETS ${KMINDEX_SERVER}
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         COMPONENT ${KMINDEX_LIB}_Runtime
)

