# 声明要求的cmake最低版本
cmake_minimum_required( VERSION 2.8 )

# 添加c++11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11 -g" )

# 声明一个cmake工程
project( mercury )

# 头文件
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# 生成库
add_library( ${PROJECT_NAME} 
    ${PROJECT_SOURCE_DIR}/src/sys/system_error.cpp
    ${PROJECT_SOURCE_DIR}/src/net/protocol.cpp
    ${PROJECT_SOURCE_DIR}/src/net/inet_address.cpp
    ${PROJECT_SOURCE_DIR}/src/net/socket_base.cpp
)

# 设置项目根目录
set( PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR} )

ENABLE_TESTING()

# 构建测试子目录
add_subdirectory(test)

# 构建tools子目录
# add_subdirectory(tools)