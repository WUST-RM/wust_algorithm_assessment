cmake_minimum_required(VERSION 3.20)
project(task6)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR})


add_executable(task6 ${PROJECT_SOURCE_DIR}/task6.cpp)
