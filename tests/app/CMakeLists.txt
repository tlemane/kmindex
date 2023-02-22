add_test(
  NAME kmindex:pa
  COMMAND ${CMAKE_SOURCE_DIR}/tests/app/run_test_pa.sh ${CMAKE_BINARY_DIR}/app/kmindex/kmindex .
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
)

add_test(
  NAME kmindex:abs
  COMMAND ${CMAKE_SOURCE_DIR}/tests/app/run_test_abs.sh ${CMAKE_BINARY_DIR}/app/kmindex/kmindex .
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
)
