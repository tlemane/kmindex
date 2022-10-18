add_library(zlib INTERFACE)

option(FORCE_BUILD_ZLIB "Always build, instead of using the system version." OFF)
set(ZLIB_MIN_REQUIRED_VERSION "1.2.1")

if (NOT FORCE_BUILD_ZLIB AND NOT FORCE_BUILD_ALL)
  find_package(ZLIB ${ZLIB_MIN_REQUIRED_VERSION})

  if (ZLIB_FOUND)
    target_link_libraries(zlib INTERFACE ZLIB::ZLIB)
    return()
  else()
    message(STATUS "ZLIB not found: use bundled version.")
  endif()

endif()

set(ZLIB_URL "https://github.com/madler/zlib/archive/refs/tags/v1.2.12.tar.gz")
set(ZLIB_HASH "db5b7326d4e0dbcbd1981b640d495c9b")
set(ZLIB_DIR ${EXT_INSTALL_DIR}/zlib)

FetchContent_Declare(
  ZLIB-fc
  URL ${ZLIB_URL}
  URL_HASH MD5=${ZLIB_HASH}
)
FetchContent_MakeAvailable(ZLIB-fc)

include(ExternalProject)
ExternalProject_Add(
  ZLIB-external
  PREFIX ZLIB-external
  SOURCE_DIR ${CMAKE_BINARY_DIR}/ext-src/zlib-fc-src
  EXCLUDE_FROM_ALL
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  BUILD_IN_SOURCE true
  CONFIGURE_COMMAND ./configure
  BUILD_COMMAND make
  INSTALL_COMMAND make prefix=${ZLIB_DIR} install
)

target_include_directories(zlib INTERFACE
  "${ZLIB_DIR}/include"
)

target_link_libraries(zlib INTERFACE
  "${ZLIB_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

add_dependencies(zlib ZLIB-external)

