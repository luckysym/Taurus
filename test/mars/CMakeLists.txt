# 声明要求的cmake最低版本
cmake_minimum_required( VERSION 2.8 )

# 添加c++11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11 -g" )

# 构建tools子目录
add_subdirectory(matrix)
add_subdirectory(elimination)
