
include(FetchContent)

set(KMCOMP_BUILD_MAIN OFF CACHE BOOL "Disable CLI program compilation" FORCE)
set(KMCOMP_METRICS OFF CACHE BOOL "Disable metric records" FORCE)

if(PORTABLE_BUILD)
    set(KMCOMP_PORTABLE_BUILD ON CACHE BOOL "Portable build required for kmcomp" FORCE)
endif()

FetchContent_Declare(kmcomp_external
  GIT_REPOSITORY "https://github.com/AlixRegnier/kmcomp.git"
  GIT_TAG "v0.1"
)
FetchContent_MakeAvailable(kmcomp_external)
