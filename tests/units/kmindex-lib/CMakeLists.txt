
add_executable(kmindex-lib-tests
  "main.cpp"
  "mer.cpp"
  "index/kindex.cpp"

)

target_link_libraries(kmindex-lib-tests gtest pthread kmindex-lib)
