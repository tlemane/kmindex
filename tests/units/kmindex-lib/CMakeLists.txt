add_executable(kmindex-lib-tests
  "main.cpp"
  "mer.cpp"
)

target_link_libraries(kmindex-lib-tests PUBLIC gtest pthread kmindex-lib fmt)

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0.0)
  target_link_libraries(kmindex-lib-tests PUBLIC stdc++fs)
endif()

add_test(
  NAME kmindex:lib
  COMMAND kmindex-lib-tests
)
set_property(
  TEST kmindex:lib
  PROPERTY ENVIRONMENT "KMINDEX_TEST_DATA=${CMAKE_SOURCE_DIR}/tests/data"
)
