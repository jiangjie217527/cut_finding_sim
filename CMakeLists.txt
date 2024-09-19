cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(my_extension LANGUAGES CXX)

add_library(my_extension SHARED ext.cpp)

target_link_libraries(my_extension PRIVATE ${TORCH_LIBRARIES} ${OpenCV_LIBS})  # 如果你的项目需要 OpenCV，你可以添加 ${OpenCV_LIBS}

target_include_directories(my_extension PRIVATE ${TORCH_INCLUDE_DIRS})

set_property(TARGET my_extension PROPERTY CXX_STANDARD 14)  # 设置 C++ 标准

# 如果你的项目需要 CUDA 支持，你可以添加以下内容
# find_package(CUDA)
# if(CUDA_FOUND)
#   target_compile_definitions(my_extension PRIVATE WITH_CUDA)
#   target_link_libraries(my_extension PRIVATE ${CUDA_LIBRARIES})
# endif()

# 如果你的项目需要特定的 CUDA 架构版本，你可以添加以下内容
# set_property(TARGET my_extension PROPERTY CUDA_ARCHITECTURES 70 75 80)

# 安装规则
install(TARGETS my_extension LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# 如果你的项目有头文件，你也可以添加以下内容来安装头文件
# install(FILES my_extension.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
