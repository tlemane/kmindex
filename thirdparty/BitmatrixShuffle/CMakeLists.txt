if (STATIC_BUILD)
    set(FORCE_STATIC_ZSTD ON)
endif()

FetchContent_Declare(bitmatrix_shuffle_ext
  GIT_REPOSITORY "https://github.com/AlixRegnier/BitmatrixShuffle"
  GIT_TAG        "main"
)
FetchContent_MakeAvailable(bitmatrix_shuffle_ext)
target_compile_options(bitmatrixshuffle PRIVATE -mavx -mavx2)
