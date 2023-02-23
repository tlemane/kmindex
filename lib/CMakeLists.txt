configure_file("${CMAKE_SOURCE_DIR}/lib/config.hpp.in" "${CMAKE_BINARY_DIR}/include/kmindex/config.hpp")

set(KMINDEX_LIB "kmindex-lib")
if (NOT APPLE)
  set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()


file(GLOB_RECURSE KMINDEX_SOURCES "src/*.cpp")
add_library(${KMINDEX_LIB} ${KMINDEX_SOURCES})

set_target_properties(${KMINDEX_LIB} PROPERTIES OUTPUT_NAME "kmindex")
target_compile_definitions(${KMINDEX_LIB} PUBLIC KMTRICKS_PUBLIC)

add_library(${KMINDEX_LIB}::${KMINDEX_LIB} ALIAS ${KMINDEX_LIB})

target_compile_features(${KMINDEX_LIB} PUBLIC cxx_std_17)

target_link_libraries(${KMINDEX_LIB} PUBLIC
  $<BUILD_INTERFACE:build_type_flags>
  $<BUILD_INTERFACE:warning_flags>
  # $<BUILD_INTERFACE:asan_flags>
  $<BUILD_INTERFACE:fmt>
  $<BUILD_INTERFACE:kmtricks>
  $<BUILD_INTERFACE:json>
  $<BUILD_INTERFACE:mio_mmap>
  $<BUILD_INTERFACE:xxhash>
  $<BUILD_INTERFACE:sha1h>
  $<BUILD_INTERFACE:semver>
  $<BUILD_INTERFACE:bitpack>
)

add_dependencies(${KMINDEX_LIB} deps)

target_include_directories(${KMINDEX_LIB} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(${KMINDEX_LIB} PROPERTIES
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  VERSION ${CMAKE_PROJECT_VERSION}
)

#include(CMakePackageConfigHelpers)
#include(GenerateExportHeader)
#generate_export_header(${KMINDEX_LIB} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/${KMINDEX_LIB}/${KMINDEX_LIB}_export.hpp)
#
#set(exported_targets "${CMAKE_BINARY_DIR}/${KMINDEX_LIB}-targets")
#set(exported_name "${KMINDEX_LIB}-targets")
#set(pkg_config "${CMAKE_BINARY_DIR}/${KMINDEX_LIB}.pc")
#set(project_config "${CMAKE_BINARY_DIR}/${KMINDEX_LIB}-config.cmake")
#set(version_config "${CMAKE_BINARY_DIR}/${KMINDEX_LIB}-config-version.cmake")
#
#configure_package_config_file(
#  ${CMAKE_SOURCE_DIR}/cmake/config.cmake.in
#  ${project_config}
#  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${KMINDEX_LIB}
#)
#
#write_basic_package_version_file(
#  ${version_config}
#  VERSION ${CMAKE_PROJECT_VERSION}
#  COMPATIBILITY SameMajorVersion
#)
#
#configure_file("${PROJECT_SOURCE_DIR}/cmake/${KMINDEX_LIB}.pc.in" "${pkg_config}" @ONLY)
#
#export (
#  TARGETS ${KMINDEX_LIB} build_type_flags warning_flags asan_flags fmt kmtricks json nlohmann_json mio_mmap mio xxhash spdlog
#  NAMESPACE "${KMINDEX_LIB}::"
#  FILE ${exported_targets}.cmake
#)
#
#include(GNUInstallDirs)
#
#install (TARGETS ${KMINDEX_LIB}
#  EXPORT ${exported_name}
#  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#          COMPONENT ${KMINDEX_LIB}_Runtime
#  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#          COMPONENT ${KMINDEX_LIB}_Runtime
#          NAMELINK_COMPONENT ${KMINDEX_LIB}_Development
#  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#          COMPONENT ${KMINDEX_LIB}_Development
#)
#
#install(
#  DIRECTORY "${CMAKE_SOURCE_DIR}/lib/include/kmindex"
#            "${CMAKE_BINARY_DIR}/include/kmindex"
#  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#  COMPONENT ${KMINDEX_LIB}_Development
#  FILES_MATCHING PATTERN "*.hpp"
#)
#
#install(
#  FILES "${pkg_config}"
#  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
#)
#
#install(
#  FILES ${project_config} ${version_config}
#  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${KMINDEX_LIB}"
#)
#
#install(
#  EXPORT "${exported_name}"
#  NAMESPACE "${KMINDEX_LIB}::"
#  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${KMINDEX_LIB}"
#)

