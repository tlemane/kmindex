include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_support OUTPUT error)

if (ipo_support)
  message(STATUS "LTO enabled.")
else()
  message(STATUS "LTO not supported: <${error}>.")
endif()

if (WITH_APP)
  add_subdirectory(kmindex)
endif()

if (WITH_SERVER)
  add_subdirectory(kmindex-server)
endif()
