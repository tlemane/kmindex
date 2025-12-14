include(ExternalProject)

add_library(zstd_ext INTERFACE)
set(ZSTD_DIR ${EXT_INSTALL_DIR}/zstd)

include(ExternalProject)

ExternalProject_Add(ZSTD-external
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.7
    SOURCE_SUBDIR build/cmake
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ZSTD_DIR}
        -DZSTD_BUILD_STATIC=ON
        -DZSTD_BUILD_SHARED=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DZSTD_BUILD_PROGRAMS=OFF
        -DZSTD_BUILD_TESTS=OFF
)


target_link_libraries(zstd_ext INTERFACE "${ZSTD_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}")
target_include_directories(zstd_ext INTERFACE "${ZSTD_DIR}/include")
add_dependencies(zstd_ext ZSTD-external)
