if (WITH_APP)
  add_subdirectory(kmindex)
endif()

if (WITH_SERVER)
  add_subdirectory(kmindex-server)
endif()
