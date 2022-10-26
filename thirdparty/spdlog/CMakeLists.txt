add_library(spdlog INTERFACE)

option(FORCE_BUILD_SPDLOG "Always build, instead of using the system version." OFF)
option(SPDLOG_HEADER_ONLY "Use header only version of spdlog." OFF)

set(SPDLOG_MIN_REQUIRED_VERSION "1.8.0")

if (NOT FORCE_BUILD_SPDLOG AND NOT FORCE_BUILD_ALL)

  find_package(spdlog ${SPDLOG_MIN_REQUIRED_VERSION} CONFIG QUIET)

  if (TARGET spdlog::spdlog)

    get_target_property(SPDLOG_INCLUDE_DIRS spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)

    find_library(SPDLOG_LIBRARY spdlog)

    file(READ "${SPDLOG_INCLUDE_DIRS}/spdlog/version.h" SPD_FILE)
    string(REGEX MATCH "SPDLOG_VER_MAJOR ([0-9]*)" _ ${SPD_FILE})
    set(SPDLOG_VERSION_MAJOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "SPDLOG_VER_MINOR ([0-9]*)" _ ${SPD_FILE})
    set(SPDLOG_VERSION_MINOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "SPDLOG_VER_PATCH ([0-9]*)" _ ${SPD_FILE})
    set(SPDLOG_VERSION_PATCH ${CMAKE_MATCH_1})

    set(SPDLOG_VERSION "${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR}.${SPDLOG_VERSION_PATCH}")

    if (SPDLOG_HEADER_ONLY)
      target_link_libraries(spdlog INTERFACE spdlog::spdlog_header_only)
      message(STATUS "Found SPDLOG: header-only (found suitable version \"${SPDLOG_VERSION}\", minimum required is \"${SPDLOG_MIN_REQUIRED_VERSION}\")")
    else()
      target_link_libraries(spdlog INTERFACE spdlog::spdlog)
      message(STATUS "Found SPDLOG: ${SPDLOG_LIBRARY} (found suitable version \"${SPDLOG_VERSION}\", minimum required is \"${SPDLOG_MIN_REQUIRED_VERSION}\")")
    endif()

    return()

  else()
    message(STATUS "SPDLOG not found: use bundled version.")
  endif()

endif()

set(SPDLOG_URL "https://github.com/gabime/spdlog/archive/refs/tags/v1.9.2.tar.gz")
set(SPDLOG_HASH "cee7f3d31178a00791d7a22c6738df6d")
set(SPDLOG_DIR ${EXT_INSTALL_DIR}/spdlog)

include(ExternalProject)
ExternalProject_Add(
  SPDLOG-external
  PREFIX SPDLOG-external
  URL ${SPDLOG_URL}
  URL_MD5 ${LZ4_HASH}
  EXCLUDE_FROM_ALL
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${SPDLOG_DIR}
)

target_include_directories(spdlog INTERFACE
  "${SPDLOG_DIR}/include"
)

if (SPDLOG_HEADER_ONLY)
  target_compile_definitions(spdlog INTERFACE SPDLOG_HEADER_ONLY)
else()
  target_link_libraries(spdlog INTERFACE
    "${SPDLOG_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}spdlog${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
endif()

add_dependencies(spdlog SPDLOG-external)

