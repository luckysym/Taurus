# 声明要求的cmake最低版本
cmake_minimum_required( VERSION 2.8 )

# 添加c++11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11" )

# 声明一个cmake工程
project( taurus )

# 头文件
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_library( ${PROJECT_NAME} SHARED
    ${PROJECT_SOURCE_DIR}/src/sys/system_error.cpp
    ${PROJECT_SOURCE_DIR}/src/net/protocol.cpp
)
