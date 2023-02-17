add_library(json INTERFACE)
target_include_directories(json INTERFACE ${THIRD_DIR}/nlohmann_json/include/single_include)
