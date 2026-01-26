include(ExternalProject)

add_library(lz4_ext INTERFACE)
set(LZ4_DIR ${EXT_INSTALL_DIR}/lz4)

# Download and build Lz4 as an External Project
ExternalProject_Add(LZ4-external
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG v1.10.0
    SOURCE_SUBDIR build/cmake
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${LZ4_DIR}
        -DBUILD_STATIC_LIBS=${FORCE_STATIC_LZ4}
        -DBUILD_SHARED_LIBS=${BC_LZ4_BUILD_SHARED_LIB}
        -DLZ4_BUILD_CLI=OFF
)

target_link_libraries(lz4_ext INTERFACE "${LZ4_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX}")
target_include_directories(lz4_ext INTERFACE "${LZ4_DIR}/include")
add_dependencies(lz4_ext LZ4-external)
