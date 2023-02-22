add_executable(kmindex-lib-tests
  "main.cpp"
  "mer.cpp"
)

target_link_libraries(kmindex-lib-tests gtest pthread kmindex-lib fmt)

add_test(
  NAME kmindex:lib
  COMMAND kmindex-lib-tests
)
set_property(
  TEST kmindex:lib
  PROPERTY ENVIRONMENT "KMINDEX_TEST_DATA=${CMAKE_SOURCE_DIR}/tests/data"
)
