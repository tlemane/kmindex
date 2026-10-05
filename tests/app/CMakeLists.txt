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

add_test(
  NAME kmindex:query_batch_base
  COMMAND ${CMAKE_SOURCE_DIR}/tests/app/run_test_query_batch_base.sh ${CMAKE_BINARY_DIR}/app/kmindex/kmindex .
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
)

add_test(
  NAME kmindex:query2_memory
  COMMAND ${CMAKE_SOURCE_DIR}/tests/app/run_test_query2_memory.sh ${CMAKE_BINARY_DIR}/app/kmindex/kmindex .
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
)

add_test(
  NAME kmindex:query2_multi
  COMMAND ${CMAKE_SOURCE_DIR}/tests/app/run_test_query2_multi.sh ${CMAKE_BINARY_DIR}/app/kmindex/kmindex .
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
)

add_test(
  NAME kmindex:compress
  COMMAND ${CMAKE_SOURCE_DIR}/tests/app/run_test_cpr.sh ${CMAKE_BINARY_DIR}/app/kmindex/kmindex .
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/data
)
