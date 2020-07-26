cmake_minimum_required(VERSION 2.5)
project(netlib)

#动态库输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#设置配置参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

#包含目录下所有资源文件
aux_source_directory(. SRC_LISTS)

#生产可执行动态库.so文件
add_library(netlib SHARED ${SRC_LISTS})


