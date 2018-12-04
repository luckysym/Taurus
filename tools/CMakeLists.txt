# 声明要求的cmake最低版本
cmake_minimum_required( VERSION 2.8 )

LINK_DIRECTORIES(${PROJECT_ROOT_DIR}/build) 

# 本地IP地址枚举工具
add_subdirectory(lsnetaddr)
add_subdirectory(echoserver)
