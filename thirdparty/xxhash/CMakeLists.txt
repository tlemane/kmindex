add_library(xxhash INTERFACE)

option(FORCE_BUILD_XXHASH "Always build, instead of using the system version." OFF)
set(XXHASH_MIN_REQUIRED_VERSION "0.8.0")

if (NOT FORCE_BUILD_XXHASH AND NOT FORCE_BUILD_ALL)
  find_package(XXHASH ${XXHASH_MIN_REQUIRED_VERSION})

  if (XXHASH_FOUND)
    target_include_directories(xxhash INTERFACE ${XXHASH_INCLUDE_DIR})
    target_link_libraries(xxhash INTERFACE ${XXHASH_LIBRARY})
    return()
  else()
    message(STATUS "XXHASH not found: use bundled version.")
  endif()

endif()

set(XXHASH_URL "https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.1.tar.gz")
set(XXHASH_HASH "b67c587f5ff4894253da0095ba7ea393")
set(XXHASH_DIR ${EXT_INSTALL_DIR}/xxhash)

include(ExternalProject)
ExternalProject_Add(
  XXHASH-external
  PREFIX XXHASH-external
  #URL ${XXHASH_URL}
  #URL_MD5 ${XXHASH_HASH}
  GIT_REPOSITORY "https://github.com/Cyan4973/xxHash"
  GIT_TAG dev
  EXCLUDE_FROM_ALL
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  BUILD_IN_SOURCE true
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make
  INSTALL_COMMAND make PREFIX=${XXHASH_DIR} install
)

target_include_directories(xxhash SYSTEM INTERFACE
  "${XXHASH_DIR}/include"
)

target_link_libraries(xxhash INTERFACE
  "${XXHASH_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}xxhash${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

add_dependencies(xxhash XXHASH-external)
