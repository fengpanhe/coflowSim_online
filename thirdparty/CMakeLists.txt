# 设置输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

aux_source_directory(. DIR_SRCS_)
aux_source_directory(./spdlog DIR_SRCS_SPDLOG)
aux_source_directory(./rapidjson DIR_SRCS_RAPIDJSON)

add_library(spdlog SHARED ${DIR_SRCS_SPDLOG})
add_library(rapidjson SHARED ${DIR_SRCS_RAPIDJSON})