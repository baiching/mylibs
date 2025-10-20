cmake_minimum_required(VERSION 3.31)
project(mylibs C)

set(CMAKE_C_STANDARD 11)

add_library(mylibs STATIC library.c
        sockets/xxx.c)
