cmake_minimum_required(VERSION 3.25)

project(dhdylan_Sockets)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(sockets)

add_executable(example example.cpp)

target_link_libraries(example PRIVATE dhdylan::Sockets)

add_compile_options(example PRIVATE -Wall -g)