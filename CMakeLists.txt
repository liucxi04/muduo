cmake_minimum_required(VERSION 3.0)
project(muduo)

# muduo 最终编译成 so 动态库
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 定义参与编译的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./muduo/base BASE_LIST)
aux_source_directory(./muduo/net NET_LIST)
# 编译生成动态库 muduo
add_library(muduo SHARED ${SRC_LIST} ${BASE_LIST} ${NET_LIST})