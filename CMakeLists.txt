

cmake_minimum_required(VERSION 3.2)
project(QImageWidget)

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt5Widgets)

include_directories(${PROJECT_ROOT}/include)
add_subdirectory(src)
add_subdirectory(example)

