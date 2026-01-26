add_subdirectory(lz4)
add_subdirectory(zstd)
add_subdirectory(sdsl)

add_library(simde INTERFACE)
target_include_directories(simde SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/simde)

add_library(blockcompressor)
target_sources(blockcompressor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/BlockCompressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/BlockDecompressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/zstd/BlockCompressorZSTD.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/zstd/BlockDecompressorZSTD.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/lz4/BlockCompressorLZ4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/lz4/BlockDecompressorLZ4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/src/ConfigurationLiterate.cpp
)
target_include_directories(blockcompressor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/include/zstd
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor/include/lz4
)
target_link_libraries(blockcompressor PUBLIC zstd_ext lz4_ext sdsl-lite)

add_library(bitmatrixshuffle)
target_sources(bitmatrixshuffle PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmatrixShuffle/src/fast_median.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmatrixShuffle/src/distance_matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmatrixShuffle/src/rng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmatrixShuffle/src/bitmatrixshuffle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmatrixShuffle/src/tsp.cpp
)

if (ARCH_X86_64 AND COMPILER_SUPPORTS_SSE2)
    target_compile_options(bitmatrixshuffle PUBLIC -msse2)
endif()

if (ARCH_X86_64 AND COMPILER_SUPPORTS_AVX AND NOT PORTABLE_BUILD)
    target_compile_options(bitmatrixshuffle PUBLIC -mavx)
endif()

if (ARCH_X86_64 AND COMPILER_SUPPORTS_AVX2 AND NOT PORTABLE_BUILD)
    target_compile_options(bitmatrixshuffle PUBLIC -mavx2)
endif()

target_include_directories(bitmatrixshuffle PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmatrixShuffle/include
)
target_link_libraries(bitmatrixshuffle PUBLIC blockcompressor simde)

add_library(compression_features INTERFACE)
target_link_libraries(compression_features INTERFACE blockcompressor bitmatrixshuffle zstd_ext lz4_ext)
