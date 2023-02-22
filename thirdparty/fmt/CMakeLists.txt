add_library(fmt INTERFACE)

option(FORCE_BUILD_FMT "Always build, instead of using the system version." OFF)
option(FMT_HEADER_ONLY "Use header only version of fmt." OFF)
set(FMT_MIN_REQUIRED_VERSION "8.1.0...<9.0.0")

if (NOT FORCE_BUILD_FMT AND NOT FORCE_BUILD_ALL)

  find_package(fmt ${FMT_MIN_REQUIRED_VERSION} CONFIG QUIET)

  if (TARGET fmt::fmt)

    get_target_property(FMT_INCLUDE_DIRS fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)

    find_library(FMT_LIBRARY fmt)

    file(READ ${FMT_INCLUDE_DIRS}/fmt/core.h fmt_core)

    if (NOT fmt_core MATCHES "FMT_VERSION ([0-9]+)([0-9][0-9])([0-9][0-9])")
      set(FMT_VERSION "Unknown version")
    endif()

    set(FMT_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")

    if (FMT_HEADER_ONLY)
      target_link_libraries(fmt INTERFACE fmt::fmt-header-only)
      message(STATUS "Found FMT: header-only (found suitable version \"${FMT_VERSION}\", minimum required is \"${FMT_MIN_REQUIRED_VERSION}\")")
    else()
      target_link_libraries(fmt INTERFACE fmt::fmt)
      message(STATUS "Found FMT: ${FMT_LIBRARY} (found suitable version \"${FMT_VERSION}\", minimum required is \"${FMT_MIN_REQUIRED_VERSION}\")")
    endif()

    return()

  else()
      message(STATUS "FMT not found: use bundled version.")
  endif()

endif()

set(FMT_URL "https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.tar.gz")
set(FMT_HASH "21fac48cae8f3b4a5783ae06b443973a")
set(FMT_DIR ${EXT_INSTALL_DIR}/fmt)

include(ExternalProject)
ExternalProject_Add(
  FMT-external
  PREFIX FMT-external
  URL ${FMT_URL}
  URL_MD5 ${FMT_HASH}
  EXCLUDE_FROM_ALL
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  BUILD_IN_SOURCE true
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${FMT_DIR}
)

target_include_directories(fmt SYSTEM INTERFACE
  "${FMT_DIR}/include"
)

if (NOT FMT_HEADER_ONLY)
  target_link_libraries(fmt INTERFACE
    "${FMT_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}fmt${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
else()
  target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY)
endif()

add_dependencies(fmt FMT-external)

