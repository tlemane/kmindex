cmake_policy(SET CMP0135 NEW)
set(THIRD_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

set(EXT_INSTALL_DIR ${CMAKE_BINARY_DIR}/ext-install)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/ext-src")

include(GNUInstallDirs)

add_library(bcli_lib INTERFACE)
target_include_directories(bcli_lib SYSTEM INTERFACE ${THIRD_DIR}/bcli/include)

add_subdirectory(fmt)
add_subdirectory(kmtricks)
add_subdirectory(nlohmann_json)

add_library(mio_mmap INTERFACE)

set(mio.installation OFF)
set(INSTALL_SUBPROJECTS OFF)

add_subdirectory(mio)

target_link_libraries(mio_mmap INTERFACE mio::mio)

add_subdirectory(gtest)
add_subdirectory(xxhash)

add_subdirectory(zlib)

add_library(kseqpp INTERFACE)
include(ExternalProject)

ExternalProject_Add(
  KSEQPP
  PREFIX KSEQPP
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/kseqpp
  INSTALL_COMMAND ""
  LOG_CONFIGURE ON
  LOG_BUILD ON
)

target_include_directories(kseqpp INTERFACE ${THIRD_DIR}/kseqpp/include ${THIRD_DIR}/kseqpp/include/kseq++)

add_subdirectory(spdlog)

add_library(sws INTERFACE)
ExternalProject_Add(
  SWS
  PREFIX SWS
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/Simple-Web-Server
  CMAKE_ARGS -DUSE_OPENSSL=OFF
  INSTALL_COMMAND ""
  LOG_CONFIGURE ON
  LOG_BUILD ON
)

target_include_directories(sws SYSTEM INTERFACE ${THIRD_DIR}/Simple-Web-Server/)

add_library(sha1h INTERFACE)
target_include_directories(sha1h INTERFACE ${THIRD_DIR}/sha1)

add_library(semver INTERFACE)
target_include_directories(semver INTERFACE ${THIRD_DIR}/semver/include)

add_library(deps INTERFACE)
add_dependencies(deps KSEQPP SWS mio_mmap xxhash zlib bcli_lib fmt kmtricks)

