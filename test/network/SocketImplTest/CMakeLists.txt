# 声明要求的cmake最低版本
cmake_minimum_required( VERSION 2.8 )

# 添加c++11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11 -g" )

# 声明一个cmake工程
project( socket_impl_test )

# 生成执行程序
add_executable( ${PROJECT_NAME} 
    socket_impl_test.cpp
)

target_link_libraries(${PROJECT_NAME} mercury )
target_link_libraries(${PROJECT_NAME} cppunit)

add_test(socket_impl_test socket_impl_test)