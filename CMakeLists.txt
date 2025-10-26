cmake_minimum_required(VERSION 3.28.3)
project(mylibs C)

set(CMAKE_C_STANDARD 11)

#add_library(mylibs STATIC library.c)

add_executable(mylibs sockets/tests/test_basic_client.c
                sockets/tests/test_basic_server.c
                sockets/network.h
        sockets/tests/test_oneway_client.c
        sockets/tests/test_oneway_server.c)
