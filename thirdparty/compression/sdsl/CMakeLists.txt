include(FetchContent)

set(SDSL_BUILD_TESTS OFF CACHE BOOL "Disable building tests" FORCE)
set(SDSL_BUILD_EXAMPLES OFF CACHE BOOL "Disable building examples" FORCE)
set(SDSL_BUILD_TUTORIAL OFF CACHE BOOL "Disable building tutorials" FORCE)

# Using main branch here because v3.0.3 has bad support for CMake integration
# If specific version is needed, we can integrate it without FetchContent

FetchContent_Declare(sdsl_external
  GIT_REPOSITORY "https://github.com/xxsds/sdsl-lite.git"
)
FetchContent_MakeAvailable(sdsl_external)

if (NOT TARGET sdsl-lite)
    message(FATAL_ERROR "Failed to fetch sdsl-lite")
endif()

