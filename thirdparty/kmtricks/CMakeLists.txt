add_library(kmtricks INTERFACE)

set(KMT_URL "https://github.com/tlemane/kmtricks/releases/download/v1.2.1/kmtricks-v1.2.1-sources.tar.gz")
set(FMT_HASH "369411f4b96e3b746f0170182504a6c1")
set(KMT_DIR ${EXT_INSTALL_DIR}/kmtricks)

include(ExternalProject)
ExternalProject_Add(
  KMT-external
  PREFIX KMT-external
  URL ${KMT_URL}
  URL_MD5 ${KMT_HASH}
  EXCLUDE_FROM_ALL
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

target_include_directories(kmtricks SYSTEM INTERFACE
  "${CMAKE_BINARY_DIR}/thirdparty/kmtricks/KMT-external/src/KMT-external/include/"
  "${CMAKE_BINARY_DIR}/thirdparty/kmtricks/KMT-external/src/KMT-external-build/include"
)

target_compile_definitions(kmtricks INTERFACE KMTRICKS_PUBLIC)

add_dependencies(kmtricks KMT-external)

