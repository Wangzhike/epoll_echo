cmake_minimum_required(VERSION 3.14)
project(epoll_echo)

set(CMAKE_CXX_STANDARD 11)

add_executable(epoll_server epoll_server.cpp Epoll.cpp Epoll.h Conn.cpp Conn.h common.h)

add_executable(epoll_client epoll_client.cpp Epoll.cpp Epoll.h Client.cpp Client.h common.h)

add_definitions("-Wall -g")