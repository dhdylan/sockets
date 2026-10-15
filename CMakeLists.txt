cmake_minimum_required(VERSION 3.20)

project(sockets)

set(CMAKE_BUILD_TYPE Debug)

add_executable(sockets main.cpp)

target_compile_options(sockets PRIVATE -Wall)