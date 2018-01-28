# 设置输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

aux_source_directory(. DIR_LIB_SRCS)

add_library(spdlog SHARED ${DIR_LIB_SRCS})