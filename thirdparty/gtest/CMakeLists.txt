add_library(gtest INTERFACE)
add_library(gmock INTERFACE)

option(FORCE_BUILD_GTEST "Always build, instead of using the system version." OFF)
set(GTEST_MIN_REQUIRED_VERSION "1.0")

if (NOT FORCE_BUILD_GTEST AND NOT FORCE_BUILD_ALL)

  enable_testing()
  find_package(GTest ${FMT_MIN_REQUIRED_VERSION} CONFIG QUIET)

  if (TARGET GTest::gtest AND TARGET GTest::gmock)
    find_library(GTEST_LIBRARY gtest)
    target_link_libraries(gtest INTERFACE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    message(STATUS "Found GTEST: ${GTEST_LIBRARY} (minimum required is \"${GTEST_MIN_REQUIRED_VERSION}\")")
    return()
  else()
    message(STATUS "GTEST not found: use bundled version.")
  endif()

endif()

set(GTEST_URL "https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz")
set(GTEST_HASH "b6f100bc2a5853a48046aa168ececf84")
set(GTEST_DIR ${EXT_INSTALL_DIR}/gtest)

include(ExternalProject)
ExternalProject_Add(
  GTEST-external
  PREFIX GTEST-external
  URL ${GTEST_URL}
  URL_MD5 ${GTEST_HASH}
  EXCLUDE_FROM_ALL
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  BUILD_IN_SOURCE true
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GTEST_DIR}
)

target_include_directories(gtest SYSTEM INTERFACE
  "${GTEST_DIR}/include"
)

target_link_libraries(gtest INTERFACE
  "${GTEST_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${GTEST_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}"
  pthread
)

add_dependencies(gtest GTEST-external)
