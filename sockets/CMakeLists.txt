add_library(_sockets STATIC)

target_sources(_sockets PRIVATE
    Socket.cpp
    UDPSocket.cpp)

target_include_directories(_sockets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(dhdylan::Sockets ALIAS _sockets)