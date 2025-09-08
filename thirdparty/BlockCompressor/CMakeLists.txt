if (STATIC_BUILD)
    set(FORCE_STATIC_ZSTD ON)
endif()

FetchContent_Declare(block_compressor_ext
  GIT_REPOSITORY "https://github.com/AlixRegnier/BlockCompressor"
  GIT_TAG        "main"
)
FetchContent_MakeAvailable(block_compressor_ext)
