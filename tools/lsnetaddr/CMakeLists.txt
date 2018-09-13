# 声明要求的cmake最低版本
cmake_minimum_required( VERSION 2.8 )

# 添加c++11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11" )

# 声明一个cmake工程
project( lsnetaddr )

ADD_EXECUTABLE(${PROJECT_NAME} lsnetaddr_main.cpp )
target_link_libraries(${PROJECT_NAME} taurus )