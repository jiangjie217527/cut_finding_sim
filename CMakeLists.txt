cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(cut_simulation)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加源文件
set(SOURCES
    src/DCache.cpp
    src/DRAM.cpp
    src/entry.cpp
    src/pe.cpp
    src/scheduler.cpp
    src/utils.cpp
)

# 添加头文件路径
include_directories(src)

# 添加可执行文件
add_executable(cut_simulation ${SOURCES})
