cmake_minimum_required(VERSION 3.20)

project("BSPParser" CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(BSPParser
        BSPParser.hpp
        src/bsp.cpp
        src/bsp.hpp
)
